/*
Copyright 2013 Christopher Higgins Barrett

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "JsonParser2.h"

#include <stdio.h>
// atoi()
#include <cstdlib>

#ifdef _MSC_VER
#pragma warning(push)
// yes I'm using sprintf() in some places.  Not sprintf_s; for cross-platform
//   compatibility.
#pragma warning(disable: 4996)
#endif 

namespace Core {

JsonParser2::JsonParser2(void) {
	Reset();
}


bool JsonParser2::ParseEntireFile(FILE* file, char* fread_buffer,
																	size_t fread_buffer_size_in_elements,
																	CallbackInterface* data_callback) {
	Reset();
	
	// check for no file given
	if (file == nullptr) {
		m_error_status = ErrorStatus::kError_CantAccessData;
		NotifyOfError("ParseEntireFile() was given a NULL file pointer.  "
		              "Please give a file with read access and in translate mode "
		              "to be parsed.");
		return false;
	}
	// check for no storage destination
	if (data_callback == nullptr) {
		m_error_status = ErrorStatus::kError_CantAccessData;
		NotifyOfError("ParseEntireFile() was given a NULL CallbackInterface "
		              "pointer to store its result in.  Please provide a valid "
		              "CallbackInterface.");
		return false;
	}
	
	// if no working buffer was given, make one
	bool must_delete_buffer_after = (fread_buffer == nullptr);
	if (must_delete_buffer_after) {
		fread_buffer_size_in_elements = Core::GetSystemPageSize();
		fread_buffer = new char[fread_buffer_size_in_elements];
	}
	
	m_error_status = ErrorStatus::kNotFinished;
	while (m_parser_status < ParserStatus::kDone &&
				 m_error_status <  ErrorStatus::kError_Unspecified) {
		size_t chars_this_read = fread(fread_buffer, sizeof(char),
																	 fread_buffer_size_in_elements, file);
		// if we didn't read in as much as we wanted, check why
		if (chars_this_read != fread_buffer_size_in_elements) {
			// file reading error?
			if (ferror(file)) {
				m_error_status = ErrorStatus::kError_BadFileRead;
				NotifyOfError(nullptr);
				break;
			}
			// otherwise, we reached the end of the file.
			//   No special action here. (right?)
		}
		
		// pass our chunk of memory down to the worker function for parsing
		ParseBuffer(fread_buffer, chars_this_read, data_callback);
	}
	
	// clean up our buffer, if the user didn't give us one
	if (must_delete_buffer_after)
		delete [] fread_buffer;
	
	return m_error_status < ErrorStatus::kError_Unspecified;
}


bool JsonParser2::ParseEntireFile(const char* filepath, char* fread_buffer,
                                  size_t fread_buffer_size_in_elements,
                                  CallbackInterface* data_callback) {
    FILE* file = fopen(filepath, "rt");
    if (!file)
        return false;
        
    const bool result = ParseEntireFile(file, fread_buffer, fread_buffer_size_in_elements, data_callback);
    
    fclose(file);
    return result;
}


bool JsonParser2::ParseEntireFile(const WCHAR* filepath, char* fread_buffer,
                                  size_t fread_buffer_size_in_elements,
                                  CallbackInterface* data_callback) {
    char filepathA[1024];
    for (unsigned i = 0; filepath[i]; ++i)
        filepathA[i] = char(filepath[i]);
    
    FILE* file = fopen(filepathA, "rt");
    if (!file)
        return false;
        
    const bool result = ParseEntireFile(file, fread_buffer, fread_buffer_size_in_elements, data_callback);
    
    fclose(file);
    return result;
}


bool JsonParser2::ParseBuffer(const char* buffer, size_t buffer_size,
															CallbackInterface* data_callback) {
	// check for no buffer given
	if (buffer == nullptr) {
		m_error_status = ErrorStatus::kError_CantAccessData;
		NotifyOfError("ParseBuffer() was given a NULL buffer pointer.");
		return false;
	}
	// check for no storage destination
	if (data_callback == nullptr) {
		m_error_status = ErrorStatus::kError_CantAccessData;
		NotifyOfError("ParseBuffer() was given a NULL CallbackInterface "
		              "pointer to store its result in.  Please provide a valid "
		              "CallbackInterface.");
		return false;
	}
	
	m_source = buffer;
	m_source_size = buffer_size;
	m_source_index = 0;
	m_data_callback = data_callback;
	
	while (m_error_status < ErrorStatus::kError_Unspecified &&
				 m_parser_status != ParserStatus::kDone &&
				 m_parser_status != ParserStatus::kFinishedAllData &&
				 m_source_index < m_source_size) {
		// walk through buffer, parsing data
		switch (m_parser_status) {
			// nothing parsed yet.  Only valid thing is the root object's start.
			case ParserStatus::kNotStarted:
				SkipWhitespace(true);
				if (m_source_index >= m_source_size)
					break;
				// should have root object
				if (m_source[m_source_index] == '{')
					BeginObject();
				// if we didn't begin the root object, error
				else {
					m_error_status = ErrorStatus::kParseError_ExpectedBeginObject;
					m_parser_status = ParserStatus::kDone;
					NotifyOfError("All valid JSON data begins with the opening curly "
					              "brace of the root, unnamed object.");
				}
				break;
			
			// An object has already begun.  Only valid things are the name of the
			//   first name-value pair, or an object-terminating curly brace.
			case ParserStatus::kBeganObject:
				SkipWhitespace(true);
				if (m_source_index >= m_source_size)
					break;
				// all object fields have names
				if (m_source[m_source_index] == '"')
					BeginName();
				// objects can be empty
				else if (m_source[m_source_index] == '}')
					EndObject();
				// nothing else is valid
				else {
					m_error_status = ErrorStatus::kParseError_ExpectedString;
					m_parser_status = ParserStatus::kDone;
					NotifyOfError("Every field in an object is made up of a name-value "
					              "pair.  Like this: { \"answer\" : 42 }\n"
					              "Other possible error: Didn't terminate your empty "
					              "object properly.  Do like this: { }");
				}
				break;
			
			// An array has already just begun, or an array value-separating comma
			//   was encoutnered after a valid value in the same array.
			//   Only valid things are...
			//    - Skippable whitespace.
			//    - string value-starting double quote.
			//    - Numeric value-starting digit (or negative sign).
			//    - true-starting 't'.
			//    - false-starting 'f'.
			//    - null-starting 'n'.
			case ParserStatus::kBeganArray:
			case ParserStatus::kNeedAnotherDataElement_InArray:
			{
				SkipWhitespace(true);
				if (m_source_index >= m_source_size)
					break;
				
				switch (m_source[m_source_index])
				{
					// string value?
					case '"':
						BeginStringValue();
						break;
					// number opening negative sign?
					case '-':
						m_parser_status = ParserStatus::kNumberSawLeadingNegativeSign;
						BeginNumberValue_AtLeadingNegative();
						break;
					// number leading zero?
					case '0':
						m_parser_status = ParserStatus::kNumberSawLeadingZero;
						BeginNumberValue_AtLeadingZero();
						break;
					// number leading 1-9 digit?
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						m_parser_status = ParserStatus::kNumberReadingWholeDigits;
						//BeginNumberValue_AtNormalDigit();
						ContinueNumberValue_ReadingWholeDigits();
						break;
					// bad attempt to start a fractional number?
					case '.':
						m_parser_status = ParserStatus::kDone;
						m_error_status = ErrorStatus::kParserError_PrematureDecimalPoint;
						NotifyOfError("Numbers cannot start with a decimal point.  Begin "
						              "them with a zero first (0.123)");
						break;
					// 'true' value?
					case 't':
						BeginTrueValue();
						break;
					// 'false' value?
					case 'f':
						BeginFalseValue();
						break;
					// 'null' value?
					case 'n':
						BeginNullValue();
						break;
					// child object value?
					case '{':
						BeginObject();
						break;
					// child array value?
					case '[':
						BeginArray();
						break;
					// arrays can be empty
					case ']':
						if (m_parser_status == ParserStatus::kNeedAnotherDataElement_InArray)
						{
							m_parser_status = ParserStatus::kDone;
							m_error_status = ErrorStatus::kParseError_ExpectedValue;
							NotifyOfError("Expected another value in Array.  Got end-of-array"
							              " square bracket instead.  Either give another "
							              "value, or remove the last comma in the array.");
							break;
						}
						EndArray();
						break;
					// TODO: Implement other value types.
					default:
						m_error_status = ErrorStatus::kParseError_ExpectedValue;
						m_parser_status = ParserStatus::kDone;
						NotifyOfError("Saw an object's element's name, then the "
						              "name-value separator.  But no valid value came "
						              "after that.");
						break;
				}
				break;
			}
			
			// We've already seen a name's opening double-quote.  Since we're
			//   currently reading a name, keep reading in the name.  Until
			//   double-quotes are again encountered.
			case ParserStatus::kReadingName:
				if (m_source[m_source_index] == '"')
					FinishName();
				else
					ContinueName();
				break;
			
			// After a name (which only occurs in name-value pairs), the only thing
			//   we should see is a name-value-separating colon.
			case ParserStatus::kFinishedName:
				SkipWhitespace(true);
				if (m_source_index >= m_source_size)
					break;
				if (m_source[m_source_index] == ':') {
					m_parser_status = ParserStatus::kSawNameValueSeparator;
					++m_source_index;
					++m_current_column;
				}
				// otherwise, we have malformed data
				else {
					m_error_status = ErrorStatus::kParseError_ExpectedNameValueSeparator;
					m_parser_status = ParserStatus::kDone;
					NotifyOfError("Every name must be followed by the name-value "
					              "separator (a colon).  "
					              "Like this: { \"name\" : \"value\" }");
				}
				break;
			
			// After the name-value separater, we should see...
			//   double-quote to begin a string value
			//   digit 0-9 or the negative sign (no positive sign!) to begin a number
			//   't' in 'true'  (must be lowercase)
			//   'f' in 'false'  (must be lowercase)
			//   'n' in 'null'  (must be lowercase)
			case ParserStatus::kSawNameValueSeparator:
				SkipWhitespace(true);
				if (m_source_index >= m_source_size)
					break;
				switch (m_source[m_source_index]) {
					// string value?
					case '"':
						BeginStringValue();
						break;
					// number opening negative sign?
					case '-':
						m_parser_status = ParserStatus::kNumberSawLeadingNegativeSign;
						BeginNumberValue_AtLeadingNegative();
						break;
					// number leading zero?
					case '0':
						m_parser_status = ParserStatus::kNumberSawLeadingZero;
						BeginNumberValue_AtLeadingZero();
						break;
					// number leading 1-9 digit?
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						m_parser_status = ParserStatus::kNumberReadingWholeDigits;
						//BeginNumberValue_AtNormalDigit();
						ContinueNumberValue_ReadingWholeDigits();
						break;
					// bad attempt to start a fractional number?
					case '.':
						m_parser_status = ParserStatus::kDone;
						m_error_status = ErrorStatus::kParserError_PrematureDecimalPoint;
						NotifyOfError("Numbers cannot start with a decimal point.  Begin "
						              "them with a zero first (0.123)");
						break;
					// 'true' value?
					case 't':
						BeginTrueValue();
						break;
					// 'false' value?
					case 'f':
						BeginFalseValue();
						break;
					// 'null' value?
					case 'n':
						BeginNullValue();
						break;
					// child object value?
					case '{':
						BeginObject();
						break;
					// child array value?
					case '[':
						BeginArray();
						break;
					// TODO: Implement other value types.
					default:
						m_error_status = ErrorStatus::kParseError_ExpectedValue;
						m_parser_status = ParserStatus::kDone;
						NotifyOfError("Saw an object's element's name, then the "
						              "name-value separator.  But no valid value came "
						              "after that.");
						break;
				}
				break;
			
			// We've already seen a string value's opening double-quote.  Since we're
			//   currently reading a string, keep reading in the string.  Until
			//   double-quotes are encountered again.  This is broken up in this way
			//   because the string may span a number of buffer-parse calls.
			case ParserStatus::kReadingStringValue:
				if (m_source[m_source_index] == '"')
					FinishStringValue();
				else
					ContinueStringValue();
				break;
			
			case ParserStatus::kReadingName_EscapedChar:
			case ParserStatus::kReadingStringValue_EscapedChar:
				HandleEscapedCharacter();
				break;
			
			case ParserStatus::kNumberSawLeadingNegativeSign:
			{
				ContinueNumberValue_AfterLeadingNegative();
				break;
			}
			
			case ParserStatus::kNumberSawLeadingZero:
			{
				ContinueNumberValue_AfterLeadingZero();
				/*
				// begin floating-pointer number?
				if (m_source[m_source_index] == '.')
				{
				}
				// end number at just zero?
				else if (m_source[m_source_index] == ',' ||
						m_source[m_source_index] == '}' ||
						m_source[m_source_index] == ']' ||
						IsWhitespace(m_source[m_source_index], true)
				{
				}
				else
				{
				}
				//*/
				break;
			}
			
			case ParserStatus::kNumberReadingWholeDigits:
			{
				switch (m_source[m_source_index])
				{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					{
						ContinueNumberValue_ReadingWholeDigits();
						break;
					}
					
					case '.':
					{
						m_parser_status = ParserStatus::kNumberSawDecimalPoint;
						m_temp_data[m_temp_data_index] = '.';
						++m_temp_data_index;
						++m_source_index;
						++m_current_column;
						break;
					}
					
					case ',':
					case '}':
					case ']':
					{
						m_parser_status = ParserStatus::kFinishedValue;
						FinishNumberValueIntegral();
						break;
					}
					
					case 'e':
					case 'E':
					{
						m_error_status =
								ErrorStatus::kParseError_ExpectedDigitOrDecimalOrEndOfNumber;
						m_parser_status = ParserStatus::kDone;
						NotifyOfError("Exponents are not supported.");
						break;
					}
					
					default:
					{
						if (IsWhitespace(m_source[m_source_index], true))
						{
							m_parser_status = ParserStatus::kFinishedValue;
							FinishNumberValueIntegral();
						}
						else
						{
							m_error_status =
									ErrorStatus::kParseError_ExpectedDigitOrDecimalOrEndOfNumber;
							m_parser_status = ParserStatus::kDone;
							NotifyOfError("Was reading integral digits in a number.  Expected"
							              " more digits, decimal point, or end of number by "
							              "\'}\', \']\', or \',\'.");
						}
						break;
					}
				} // end sub-switch
				break;
			} // end case
			
			case ParserStatus::kNumberSawDecimalPoint:
			{
				if (m_source[m_source_index] < '0' ||
				    m_source[m_source_index] > '9')
				{
					m_error_status = ErrorStatus::kParserError_UnfinishedFractionalNumber;
					m_parser_status = ParserStatus::kDone;
					NotifyOfError("Fractional numbers must have digits after the decimal "
					              "point.  So \"0.\" is not valid, but \"0.0\" is.");
					break;
				}
				
				ContinueNumberValue_ReadingFractionalDigits();
				m_parser_status = ParserStatus::kNumberReadingFractionalDigits;
				break;
			}
			
			case ParserStatus::kNumberReadingFractionalDigits:
			{
				if (IsWhitespace(m_source[m_source_index], true) ||
				    m_source[m_source_index])
				{
					FinishNumberValueWithFractional();
					break;
				}
				else if ((m_source[m_source_index] < '0' ||
				          m_source[m_source_index] > '9') &&
				         m_source[m_source_index] != ']' &&
				         m_source[m_source_index] != '}' &&
				         m_source[m_source_index] != ',')
				{
					m_error_status = ErrorStatus::kParseError_ExpectedDigitOrEndOfNumber;
					m_parser_status = ParserStatus::kDone;
					NotifyOfError("Fractional portion of number terminated incorrectly.  "
					              "Should end in whitespace, array-finishing ']', "
					              "object-finishing '}', or value-separating ','.");
					break;
				}
				ContinueNumberValue_ReadingFractionalDigits();
				break;
			}
			
			case ParserStatus::kReadingTrueValue:
			{
				if (m_temp_data_index < 4)
					ContinueTrueValue();
				else
					FinishTrueValue();
				break;
			}
			
			case ParserStatus::kReadingFalseValue:
			{
				if (m_temp_data_index < 4)
					ContinueFalseValue();
				else
					FinishFalseValue();
				break;
			}
			
			//case ParserStatus::kReadingFalseValue:
				//break;
			
			case ParserStatus::kReadingNullValue:
			{
				if (m_temp_data_index < 4)
					ContinueNullValue();
				else
					FinishNullValue();
				break;
			}
			
			// Just finished a value (of any kind).  We should see...
			//   If currently in an object:
			//    - whitespace that can be ignored.
			//    - value-separating comma.
			//    - object-terminating curly brace.
			//   If currently in an array:
			//    - whitespace that can be ignored.
			//    - value-separating comma.
			//    - array-terminating square bracket.
			case ParserStatus::kFinishedValue:
			{
				SkipWhitespace(true);
				if (m_source_index >= m_source_size)
					break;
				// value-separating comma?
				if (m_source[m_source_index] == ',') {
					ClearNameAndDataBuffers();
					++m_source_index;
					++m_current_column;
					if (m_object_type_stack[m_object_type_stack_index - 1] == true)
						m_parser_status = ParserStatus::kNeedAnotherDataElement_InObject;
					else
						m_parser_status = ParserStatus::kNeedAnotherDataElement_InArray;
				}
				// object-terminating curly brace?
				else if (m_source[m_source_index] == '}') {
					EndObject();
				}
				// array-terminating square bracket?
				else if (m_source[m_source_index] == ']') {
					EndArray();
				}
				// otherwise, unexpected invalid data
				else {
					m_error_status =
							ErrorStatus::kParseError_ExpectedValueSeparatorOrEndOfContainer;
					m_parser_status = ParserStatus::kDone;
					// were we in an object?
					if (m_object_type_stack[m_object_type_stack_index - 1] == true) {
						NotifyOfError("Every name-value pair in an object must be "
						              "followed by either the name-value separating "
						              "comma (,), or the termination of the containing "
						              "object (})");
					}
					// otherwise, we were in an array
					else {
						NotifyOfError("Every value in an array must be followed by either "
						              "the value separating comma (,), or the "
						              "termination of the containing array (])");
					}
				}
				break;
			}
			
			// Just got a value-separating comma.  Only valid things are...
			//    - Skippable whitespace.
			//    - Name-starting double quotes.
			case ParserStatus::kNeedAnotherDataElement_InObject:
			{
				SkipWhitespace(true);
				if (m_source_index >= m_source_size)
					break;
				// all object fields have names, or it could be an array's string
				if (m_source[m_source_index] == '"')
					BeginName();
				// can't end the object when we're looking for another value
				else if (m_source[m_source_index] == '}') {
					m_parser_status = ParserStatus::kDone;
					m_error_status = ErrorStatus::kParseError_ExpectedString;
					NotifyOfError("Ended object too early.  Already saw a comma, which "
					              "means another item is expected.  "
					              "Like this: { \"item1\" : 1, \"item2\" : 2 }");
				}
				// nothing else is valid
				else {
					m_parser_status = ParserStatus::kDone;
					m_error_status = ErrorStatus::kParseError_ExpectedString;
					NotifyOfError("After a value-separating comma in an object, "
					              "the next element must be a name-value pair.  "
					              "Like this: { \"foo\" : 1, \"bar\" : 2 }");
				}
				break;
			}
			
			// TODO: Check for more data, and error if more is encountered.
			//   More data after all is finished probably means the user has
			//   mis-matching braces.  Or more than one root object.
			case ParserStatus::kFinishedAllData:
			{
				m_source_index = m_source_size;
				m_parser_status = ParserStatus::kDone;
				break;
			}
		}
	}
	
	if (m_error_status == ErrorStatus::kNotStarted) {
		m_error_status = ErrorStatus::kDone;
		m_parser_status = ParserStatus::kDone;
	}
	return m_error_status < ErrorStatus::kError_Unspecified;
}


void JsonParser2::Reset(void) {
	m_error_status = ErrorStatus::kNotStarted;
	m_parser_status = ParserStatus::kNotStarted;
	m_temp_name[0] = '\0';
	m_temp_data[0] = '\0';
	m_temp_name_index = 0;
	m_temp_data_index = 0;
	//m_data_callback = nullptr;
	m_source_size = 0;
	m_source_index = 0;
	
	m_current_row = 1;
	m_current_column = 1;
	
	m_object_type_stack_index = 0;
}


void JsonParser2::NotifyOfError(const char* message) {
	fprintf(stderr, "  JsonParser2 error: (row %d, col %d)\n",
			m_current_row, m_current_column);
	// self-described error
	if (message)
		fprintf(stderr, "%s\nJsonParser2 Status: ", message);
	// status-based message
	switch (m_error_status) {
		case ErrorStatus::kError_CantAccessData:
			fprintf(stderr, "Can't access data.");
			break;
	}
	fprintf(stderr, "\n\n");
}


bool JsonParser2::IsWhitespace(char c, bool newlines_count) const {
	if (newlines_count)
	{
		switch (c)
		{
			case ' ':
			case 0x09: // TAB (horizontal tab)
			case 0x0A: // LF  (NL line feed, new line)
			case 0x0D: // CR  (carriage return)
				return true;
		}
		// break not needed
	}
	// if newlines don't count, only space and TAB are accepted
	else if (c == ' ' || c == 0x09)
	{
		return true;
	}
	
	return false;
}


void JsonParser2::SkipWhitespace(bool also_skip_newlines) {
	while (m_source_index < m_source_size &&
				 IsWhitespace(m_source[m_source_index], also_skip_newlines))
	{
		if (m_source[m_source_index] == '\n') {
			++m_current_row;
			m_current_column = 1;
		}
		else
			++m_current_column;
		
		++m_source_index;
	}
}


void JsonParser2::BeginObject(void) {
	//SkipWhitespace(true);
	// if we have the right character, it's okay to begin the object
	//if (m_source[m_source_index] == '{') {
		// update internal status
		m_parser_status = ParserStatus::kBeganObject;
		++m_source_index;
		++m_current_column;
		// object stack tracking
		m_object_type_stack[m_object_type_stack_index] = true;
		++m_object_type_stack_index;
		// callback, if such is available
		if (m_data_callback)
			m_data_callback->BeginObject(m_temp_name, m_temp_name_index);
		//return true;
	//}
	
	//return false;
}


void JsonParser2::EndObject(void) {
	// if we're not in an object, someone ended an array with the wrong thing.
	if (m_object_type_stack[m_object_type_stack_index - 1] == false) {
		m_parser_status = ParserStatus::kDone;
		m_error_status = ErrorStatus::kParseError_ExpectedEndOfArray;
		NotifyOfError("Array terminated improperly (used curly brace).  Use the "
									"square bracket to do so instead.  Like this: [ 8, 16 ]");
		return;
	}
	
	--m_object_type_stack_index;
	// if we've run the stack out, all data is now finished.  We have a special
	//   state for this, other than kDone.  This is so if more data is
	//   encountered after, we can warn the user of mis-matching braces.
	if (m_object_type_stack_index == 0) {
		m_parser_status = ParserStatus::kFinishedAllData;
		m_error_status = ErrorStatus::kDone;
	}
	else
		m_parser_status = ParserStatus::kFinishedValue;
	++m_source_index;
	++m_current_column;
	
	// callback, if such is available
	if (m_data_callback)
		m_data_callback->EndObject();
}


void JsonParser2::BeginArray(void) {
	// update internal status
	m_parser_status = ParserStatus::kBeganArray;
	++m_source_index;
	++m_current_column;
	// object stack tracking
	m_object_type_stack[m_object_type_stack_index] = false;
	++m_object_type_stack_index;
	// callback, if such is available
	if (m_data_callback)
		m_data_callback->BeginArray(m_temp_name, m_temp_name_index);
	ClearNameAndDataBuffers();
}


void JsonParser2::EndArray(void) {
	// if we're not in an array, someone ended an object with the wrong thing.
	if (m_object_type_stack[m_object_type_stack_index - 1] == true) {
		m_parser_status = ParserStatus::kDone;
		m_error_status = ErrorStatus::kParseError_ExpectedEndOfObject;
		NotifyOfError("Object terminated improperly (used square bracket).  Use the"
									" curly brace to do so instead.  Like this: { \"foo\": 8 }");
		return;
	}
	
	--m_object_type_stack_index;
	// if we've run the stack out, all data is now finished, but something is
	//   very wrong.  The root container must be an object, not an array.
	if (m_object_type_stack_index == 0)
	{
		m_parser_status = ParserStatus::kDone;
		m_error_status = ErrorStatus::kParseError_BadStructure;
		NotifyOfError("Encountered end of all data, but root-most object was an "
		              "array.  Should have been an object.  How did you even get "
		              "to this state?");
		return;
	}
	else
		m_parser_status = ParserStatus::kFinishedValue;
	++m_source_index;
	++m_current_column;
	
	// callback, if such is available
	if (m_data_callback)
		m_data_callback->EndArray();
}


void JsonParser2::BeginName(void) { 
	// update internal status
	m_parser_status = ParserStatus::kReadingName;
	m_temp_name_index = 0;
	
	// get past the opening double-quote
	++m_source_index;
	++m_current_column;
}


void JsonParser2::ContinueName(void) {
	size_t name_len = 0;
	// read name, while watching for both end of buffer, and end of string
	while (m_source_index + name_len < m_source_size &&
	       m_source[m_source_index + name_len] != '\\' &&
				 m_source[m_source_index + name_len] != '"') {
		++name_len;
	}
	
	// copy found name into temp buffer for holding.
	//   Being careful not to overflow our internal buffer.
	size_t copy_amount = name_len;
	if (m_temp_name_index + copy_amount >= kMaxNameLength)
		copy_amount = kMaxNameLength - m_temp_name_index;
	// now copy
	memcpy(m_temp_name + m_temp_name_index, m_source + m_source_index,
				 copy_amount);
	m_temp_name_index += copy_amount;
	
	if (m_source[m_source_index + name_len] == '\\') {
		m_parser_status = ParserStatus::kReadingName_EscapedChar;
		// skip past the escape sequence-initiating backslash
		++m_source_index;
	}
	
	m_source_index += name_len;
	m_current_column += name_len;
}


void JsonParser2::FinishName(void) {
	m_temp_name[m_temp_name_index] = '\0';
	m_parser_status = ParserStatus::kFinishedName;
	++m_source_index;
	++m_current_column;
}


void JsonParser2::BeginStringValue(void) {
	// update internal status
	m_parser_status = ParserStatus::kReadingStringValue;
	m_temp_data_index = 0;
	// get past the opening double-quote
	++m_source_index;
	++m_current_column;
}


void JsonParser2::ContinueStringValue(void) {
	size_t data_len = 0;
	// read string value, while watching for both end of buffer,
	//   and end of string
	while (m_source_index + data_len < m_source_size &&
	       m_source[m_source_index + data_len] != '\\' &&
				 m_source[m_source_index + data_len] != '"') {
		++data_len;
	}
	
	// copy found string into temp buffer for holding
	//   Being careful not to overflow our internal buffer.
	size_t copy_amount = data_len;
	if (m_temp_data_index + copy_amount >= kMaxStringLength)
		copy_amount = kMaxStringLength - m_temp_data_index;
	memcpy(m_temp_data + m_temp_data_index, m_source + m_source_index,
				 copy_amount);
	m_temp_data_index += copy_amount;
	
	if (m_source[m_source_index + data_len] == '\\') {
		m_parser_status = ParserStatus::kReadingStringValue_EscapedChar;
		// skip past the escape sequence-initiating backslash
		++m_source_index;
	}
	
	m_source_index += data_len;
	m_current_column += data_len;
}


void JsonParser2::HandleEscapedCharacter(void) {
	char special_char = '\0';
	
	switch (m_source[m_source_index])
	{
		case '"':
		case '\\':
		case '/':
			special_char = m_source[m_source_index];
			break;
		
		// backspace
		case 'b':
			special_char = 0x08;
			break;
		
		// formfeed
		case 'f':
			special_char = 0x0C;
			break;
		
		// newline
		case 'n':
			special_char = 0x0A;
			break;
		
		// carriage return
		case 'r':
			special_char = 0x0D;
			break;
		
		// horizontal tab
		case 't':
			special_char = 0x09;
			break;
		
		// unicode (u is followed by 4 hexadecimal digits
		case 'u':
			m_parser_status = ParserStatus::kDone;
			m_error_status = ErrorStatus::kParseError_SixCharacterEscapeSequenceNotYetSupported;
			NotifyOfError("Six-character escape sequences are not yet supported.  An "
			              "example of this is \"\\u005C\".");
			return;
		
		default:
			m_parser_status = ParserStatus::kDone;
			m_error_status = ErrorStatus::kParseError_InvalidEscapedCharacter;
			NotifyOfError("Invalid escaped character.  The only valid ones are \\\", "
			              "\\\\, \\/, \\b, \\f, \\n, \\r, \\t.  \\uXXXX is also not "
			              "yet supported.");
			return;
	}
	
	if (m_parser_status == ParserStatus::kReadingName_EscapedChar)
	{
		if (m_temp_name_index + 1 < kMaxNameLength)
		{
			*(m_temp_name + m_temp_name_index) = special_char;
			++m_temp_name_index;
		}
		
		m_parser_status = ParserStatus::kReadingName;
	}
	else // reading string value with an escaped character
	{
		if (m_temp_data_index + 1 < kMaxStringLength)
		{
			*(m_temp_data + m_temp_data_index) = special_char;
			++m_temp_data_index;
		}
		
		m_parser_status = ParserStatus::kReadingStringValue;
	}
	
	++m_source_index;
	++m_current_column;
}


void JsonParser2::FinishStringValue(void)
{
	m_temp_data[m_temp_data_index] = '\0';
	m_parser_status = ParserStatus::kFinishedValue;
	++m_source_index;
	++m_current_column;
	// notify user of new data.  Doesn't matter if we're in an object or an
	//   array, since m_temp_name will appropriately be pointing at an empty
	//   string (not NULL pointer, but empty string) iff we're in an array.
	if (m_data_callback)
		m_data_callback->GotString(m_temp_name, m_temp_name_index,
		                           m_temp_data, m_temp_data_index);
}


void JsonParser2::BeginNumberValue_AtLeadingNegative(void)
{
	// internal status already updated by caller (parse buffer)
	m_temp_data[0] = '-';
	m_temp_data_index = 1;
	//ContinueNumberValue_AfterLeadingNegative();
	++m_source_index;
	++m_current_column;
}


void JsonParser2::BeginNumberValue_AtLeadingZero(void)
{
	// internal status already updated by caller (parse buffer)
	m_temp_data[0] = '0';
	m_temp_data_index = 1;
	//ContinueNumberValue_AfterLeadingZero();
	++m_source_index;
	++m_current_column;
}


void JsonParser2::BeginNumberValue_AtNormalDigit(void)
{
	// internal status already updated by caller (parse buffer)
	m_temp_data[0] = m_source[m_source_index];
	m_temp_data_index = 1;
	//ContinueNumberValue_ReadingWholeDigits();
}


void JsonParser2::ContinueNumberValue_AfterLeadingNegative(void)
{
	switch (m_source[m_source_index])
	{
		case '0':
		{
			m_parser_status = ParserStatus::kNumberSawLeadingZero;
			m_temp_data[1] = '0';
			m_temp_data_index = 2;
			++m_source_index;
			++m_current_column;
			break;
		}
		
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			m_parser_status =  ParserStatus::kNumberReadingWholeDigits;
			m_temp_data[1] = m_source[m_source_index];
			m_temp_data_index = 2;
			++m_source_index;
			++m_current_column;
			break;
		}
		
		default:
		{
			m_parser_status = ParserStatus::kDone;
			m_error_status = ErrorStatus::kParseError_ExpectedDigit;
			NotifyOfError("Expected 0-9 digit while reading number (just read "
			              "leading '-' sign).");
			break;
		}
	} // end switch
}


void JsonParser2::ContinueNumberValue_AfterLeadingZero(void)
{
	switch (m_source[m_source_index])
	{
		case '.':
		{
			m_parser_status = ParserStatus::kNumberSawDecimalPoint;
			m_temp_data[m_temp_data_index] = '0';
			++m_temp_data_index;
			m_temp_data[m_temp_data_index] = '.';
			++m_temp_data_index;
			++m_source_index;
			++m_current_column;
			break;
		}
		
		case ',':
		case '}':
		case ']':
		case ' ':
		{
			m_parser_status =  ParserStatus::kFinishedValue;
			FinishNumberValueZero();
			break;
		}
		
		default:
		{
			if (IsWhitespace(m_source[m_source_index], true))
			{
				m_parser_status = ParserStatus::kFinishedValue;
				FinishNumberValueZero();
			}
			else
			{
				m_parser_status = ParserStatus::kDone;
				m_error_status = ErrorStatus::kParseError_ExpectedDecimalOrEndOfNumber;
				NotifyOfError("Expected either decimal point, or end of number, after "
				              "the leading digit was a zero.");
			}
			break;
		}
	} // end switch
}


void JsonParser2::ContinueNumberValue_ReadingWholeDigits(void)
{
	size_t data_len = 0;
	// read number value, while watching for both end of buffer,
	//   and end of whole digits
	while (m_source_index + data_len < m_source_size &&
			m_source[m_source_index + data_len] >= '0' &&
			m_source[m_source_index + data_len] <= '9')
	{
		++data_len;
	}
	
	// copy found number string into temp buffer for holding.
	//   Being careful not to overflow our internal buffer.
	size_t copy_amount = data_len;
	if (m_temp_name_index + copy_amount >= kMaxStringLength)
		copy_amount = kMaxStringLength - m_temp_name_index;
	memcpy(m_temp_data + m_temp_data_index, m_source + m_source_index,
				 copy_amount);
	m_temp_data_index += copy_amount;
	
	m_source_index += data_len;
	m_current_column += data_len;
}


void JsonParser2::ContinueNumberValue_ReadingFractionalDigits(void)
{
	size_t data_len = 0;
	// read number value, while watching for both end of buffer,
	//   and end of whole digits
	while (m_source_index + data_len < m_source_size &&
			m_source[m_source_index + data_len] >= '0' &&
			m_source[m_source_index + data_len] <= '9')
	{
		++data_len;
	}
	
	// copy found number string into temp buffer for holding.
	//   Being careful not to overflow our internal buffer.
	size_t copy_amount = data_len;
	if (m_temp_name_index + copy_amount >= kMaxStringLength)
		copy_amount = kMaxStringLength - m_temp_name_index;
	memcpy(m_temp_data + m_temp_data_index, m_source + m_source_index,
				 copy_amount);
	m_temp_data_index += copy_amount;
	
	m_source_index += data_len;
	m_current_column += data_len;
}


void JsonParser2::FinishNumberValueZero(void)
{
	/*
	// we've already read "true", now check that the character immediately after
	//   it is valid (ie. whitespace, name-value separator, etc.)  As opposed
	//   to being a second 'e' character, a number, or anything like that.
	if (!IsWhitespace(m_source[m_source_index], true) &&
			m_source[m_source_index] != '}' &&
			m_source[m_source_index] != ']' &&
			m_source[m_source_index] != ',') {
		m_error_status = ErrorStatus::kParseError_BadValue;
		m_parser_status = ParserStatus::kDone;
		NotifyOfError("Typo found after reading \"true\" value.");
		return;
	}
	//*/
	
	m_parser_status = ParserStatus::kFinishedValue;
	if (m_data_callback)
	{
		m_data_callback->GotInteger(m_temp_name, m_temp_name_index, 0);
	}
}


void JsonParser2::FinishNumberValueIntegral(void)
{
	m_parser_status = ParserStatus::kFinishedValue;
	if (m_data_callback)
	{
		int value = 0;
		/*
		int exponent = 1;
		// use all but first character.  We'll check that one for a negative sign
		//   separately after.
		for (int i = m_temp_data_index - 1;  i > 0;  --i)
		{
			value += (m_temp_data[i] - '0') * exponent;
			exponent *= 10;
		}
		
		// check if first character is negative sign, or just another digit
		if (m_temp_data[0] == '-')
			value *= -1;
		else
			value += (m_temp_data[0] - '0') * exponent;
		//*/
		
		m_temp_data[m_temp_data_index] = '\0';
		value = atoi(m_temp_data);
		
		m_data_callback->GotInteger(m_temp_name, m_temp_name_index, value);
	}
}


void JsonParser2::FinishNumberValueWithFractional(void)
{
	m_parser_status = ParserStatus::kFinishedValue;
	if (m_data_callback)
	{
		float value = 0;
		/*
		int exponent = 1;
		// use all but first character.  We'll check that one for a negative sign
		//   separately after.
		for (int i = m_temp_data_index - 1;  i > 0;  --i)
		{
			value += (m_temp_data[i] - '0') * exponent;
			exponent *= 10;
		}
		
		// check if first character is negative sign, or just another digit
		if (m_temp_data[0] == '-')
			value *= -1;
		else
			value += (m_temp_data[0] - '0') * exponent;
		//*/
		
		m_temp_data[m_temp_data_index] = '\0';
		value = static_cast<float>( atof(m_temp_data) );
		
		m_data_callback->GotFloat(m_temp_name, m_temp_name_index, value);
	}
}


void JsonParser2::BeginTrueValue(void) {
	// update internal status
	m_parser_status = ParserStatus::kReadingTrueValue;
	// temp data index will be used to point into our static 'true' array, to
	//   track which character we need next
	m_temp_data_index = 1;
	// leave m_source_index where it is; we'll walk it along in ContinueTrueValue
	//   as we see more character matches for the 'null' keyword
	ContinueTrueValue();
}


void JsonParser2::ContinueTrueValue(void) {
	static const char true_value[] = "true";
	
	// read true value, while watching for both end of buffer,
	//   and end of 'true' string
	while (m_source_index + m_temp_data_index < m_source_size &&
				 m_temp_data_index < 4) {
		// check if the given characters match the 'true' keyword
		if (m_source[m_source_index + m_temp_data_index] !=
				true_value[m_temp_data_index]) {
			m_parser_status = ParserStatus::kDone;
			m_error_status =
					ErrorStatus::kParseError_ExpectedContinuationOfTrueKeyword;
			char temp_buf[64] = {'\0'};
			// TODO: Ensure this sprintf cannot possibly be exploited by bad data.
			sprintf(temp_buf, "Expected '%c' in \"true\" keyword.",
					true_value[m_temp_data_index]);
			// stupid-human use of sprintf extra safety (prevents over-read later,
			//   but not over-write from the line above)
			temp_buf[sizeof(temp_buf) / sizeof(temp_buf[0]) - 1] = '\0';
			NotifyOfError(temp_buf);
			return;
		}
		++m_temp_data_index;
		++m_current_column;
	}
	
	m_source_index += m_temp_data_index;
}


void JsonParser2::FinishTrueValue(void) {
	// we've already read "true", now check that the character immediately after
	//   it is valid (ie. whitespace, name-value separator, etc.)  As opposed
	//   to being a second 'e' character, a number, or anything like that.
	if (!IsWhitespace(m_source[m_source_index], true) &&
			m_source[m_source_index] != '}' &&
			m_source[m_source_index] != ']' &&
			m_source[m_source_index] != ',') {
		m_error_status = ErrorStatus::kParseError_BadValue;
		m_parser_status = ParserStatus::kDone;
		NotifyOfError("Typo found after reading \"true\" value.");
		return;
	}
	
	m_parser_status = ParserStatus::kFinishedValue;
	if (m_data_callback)
		m_data_callback->GotBoolean(m_temp_name, m_temp_name_index, true);
}


void JsonParser2::BeginFalseValue(void) {
	// update internal status
	m_parser_status = ParserStatus::kReadingFalseValue;
	// temp data index will be used to point into our static 'false' array, to
	//   track which character we need next
	m_temp_data_index = 1;
	// leave m_source_index where it is; we'll walk it along in ContinueFalseValue
	//   as we see more character matches for the 'null' keyword
	ContinueFalseValue();
}


void JsonParser2::ContinueFalseValue(void) {
	static const char false_value[] = "false";
	
	// read false value, while watching for both end of buffer,
	//   and end of 'false' string
	while (m_source_index + m_temp_data_index < m_source_size &&
				 m_temp_data_index < 5) {
		// check if the given characters match the 'false' keyword
		if (m_source[m_source_index + m_temp_data_index] !=
				false_value[m_temp_data_index]) {
			m_parser_status = ParserStatus::kDone;
			m_error_status =
					ErrorStatus::kParseError_ExpectedContinuationOfFalseKeyword;
			char temp_buf[64] = {'\0'};
			// TODO: Ensure this sprintf cannot possibly be exploited by bad data.
			sprintf(temp_buf, "Expected '%c' in \"false\" keyword.",
					false_value[m_temp_data_index]);
			// stupid-human use of sprintf extra safety (prevents over-read later,
			//   but not over-write from the line above)
			temp_buf[sizeof(temp_buf) / sizeof(temp_buf[0]) - 1] = '\0';
			NotifyOfError(temp_buf);
			return;
		}
		++m_temp_data_index;
		++m_current_column;
	}
	
	m_source_index += m_temp_data_index;
}


void JsonParser2::FinishFalseValue(void) {
	// we've already read "false", now check that the character immediately after
	//   it is valid (ie. whitespace, name-value separator, etc.)  As opposed
	//   to being a second 'e' character, a number, or anything like that.
	if (!IsWhitespace(m_source[m_source_index], true) &&
			m_source[m_source_index] != '}' &&
			m_source[m_source_index] != ']' &&
			m_source[m_source_index] != ',') {
		m_error_status = ErrorStatus::kParseError_BadValue;
		m_parser_status = ParserStatus::kDone;
		NotifyOfError("Typo found after reading \"false\" value.");
		return;
	}
	
	m_parser_status = ParserStatus::kFinishedValue;
	if (m_data_callback)
		m_data_callback->GotBoolean(m_temp_name, m_temp_name_index, false);
}


void JsonParser2::BeginNullValue(void) {
	// update internal status
	m_parser_status = ParserStatus::kReadingNullValue;
	// temp data index will be used to point into our static 'null' array, to
	//   track which character we need next
	m_temp_data_index = 1;
	// leave m_source_index where it is; we'll walk it along in ContinueNullValue
	//   as we see more character matches for the 'null' keyword
	ContinueNullValue();
}


void JsonParser2::ContinueNullValue(void) {
	static const char null_value[] = "null";
	
	// read null value, while watching for both end of buffer,
	//   and end of 'null' string
	while (m_source_index + m_temp_data_index < m_source_size &&
				 m_temp_data_index < 4) {
		// check if the given characters match the 'null' keyword
		if (m_source[m_source_index + m_temp_data_index] !=
				null_value[m_temp_data_index]) {
			m_parser_status = ParserStatus::kDone;
			m_error_status =
					ErrorStatus::kParseError_ExpectedContinuationOfNullKeyword;
			char temp_buf[64] = {'\0'};
			// TODO: Ensure this sprintf cannot possibly be exploited by bad data.
			sprintf(temp_buf, "Expected '%c' in \"null\" keyword.",
					null_value[m_temp_data_index]);
			// stupid-human use of sprintf extra safety (prevents over-read later,
			//   but not over-write from the line above)
			temp_buf[sizeof(temp_buf) / sizeof(temp_buf[0]) - 1] = '\0';
			NotifyOfError(temp_buf);
			return;
		}
		++m_temp_data_index;
		++m_current_column;
	}
	
	m_source_index += m_temp_data_index;
}


void JsonParser2::FinishNullValue(void) {
	// we've already read "null", now check that the character immediately after
	//   it is valid (ie. whitespace, name-value separator, etc.)  As opposed
	//   to being a third 'l' character, a number, or anything like that.
	if (!IsWhitespace(m_source[m_source_index], true) &&
			m_source[m_source_index] != '}' &&
			m_source[m_source_index] != ']' &&
			m_source[m_source_index] != ',') {
		m_error_status = ErrorStatus::kParseError_BadValue;
		m_parser_status = ParserStatus::kDone;
		NotifyOfError("Typo found after reading \"null\" value.");
		return;
	}
	
	m_parser_status = ParserStatus::kFinishedValue;
	if (m_data_callback)
		m_data_callback->GotNull(m_temp_name, m_temp_name_index);
}


void JsonParser2::ClearNameAndDataBuffers(void) {
	m_temp_name[0] = '\0';
	m_temp_name_index = 0;
	m_temp_data[0] = '\0';
	m_temp_data_index = 0;
}

} // namespace Core

#ifdef _MSC_VER
#pragma warning(pop)
#endif
