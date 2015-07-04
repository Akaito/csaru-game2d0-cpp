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

#pragma once

#include "../../Utils.h"

namespace Core {

class JsonParser2 {
 public:
	static const size_t kMaxNameLength = 28;
	static const size_t kMaxStringLength = 64;
	static const size_t kMaxDepth = 7;
	
	struct ErrorStatus {
		enum Enum {
			kNotStarted = 0,
			kNotFinished,
			kDone,
			
			// lowest actual error code.  If checking for error, check if status is
			//   greater-than-or-equal-to this code.
			kError_Unspecified,
			
			// No data buffer given, or no file given, or no DataMapMutator given.
			kError_CantAccessData,
			
			// Failed to read from the given file.
			kError_BadFileRead,
			
			// lowest parsing-based error.  This and above means your data is
			//   malformed.
			kParseError_Unspecified,
			
			kParseError_ExpectedBeginObject,
			kParseError_ExpectedEndOfObject,
			kParseError_ExpectedEndOfArray,
			kParseError_ExpectedString,
			kParseError_SixCharacterEscapeSequenceNotYetSupported,
			kParseError_InvalidEscapedCharacter,
			kParseError_ExpectedNameValueSeparator,
			kParseError_ExpectedValue,
			kParserError_PrematureDecimalPoint,
			kParserError_UnfinishedFractionalNumber,
			kParseError_ExpectedDigit,
			kParseError_ExpectedDecimalOrEndOfNumber,
			kParseError_ExpectedDigitOrDecimalOrEndOfNumber,
			kParseError_ExpectedDigitOrEndOfNumber,
			kParseError_ExpectedContinuationOfTrueKeyword,
			kParseError_ExpectedContinuationOfFalseKeyword,
			kParseError_ExpectedContinuationOfNullKeyword,
			kParseError_BadValue, // such as "nulll"
			kParseError_ExpectedValueSeparatorOrEndOfContainer,
			kParseError_BadStructure
		};
		private:
			// prevent construction of meaningless struct
			ErrorStatus();
	};
	
	struct ParserStatus {
		enum Enum {
			kNotStarted = 0,
			
			kBeganObject,
			kBeganArray,
			kReadingName,
			kReadingName_EscapedChar,
			kFinishedName,
			kSawNameValueSeparator,
			
			kReadingStringValue,
			kReadingStringValue_EscapedChar,
			
			kNumberSawLeadingNegativeSign,
			kNumberSawLeadingZero,
			kNumberReadingWholeDigits,
			kNumberSawDecimalPoint,
			kNumberReadingFractionalDigits,
			
			kReadingTrueValue,
			kReadingFalseValue,
			kReadingNullValue,
			
			kFinishedValue,
			kNeedAnotherDataElement_InObject,
			kNeedAnotherDataElement_InArray,
			
			kDone,
			kFinishedAllData
		};
		private:
			// prevent construction of meaningless struct
			ParserStatus();
	};
	
	struct CallbackInterface {
		virtual void BeginObject(const char* name, size_t name_len) = 0;
		virtual void EndObject(void) = 0;
		virtual void BeginArray(const char* name, size_t name_len) = 0;
		virtual void EndArray(void) = 0;
		virtual void GotString(const char* name, size_t name_len,
		                       const char* value, size_t value_len) = 0;
		virtual void GotFloat(const char* name, size_t name_len, float value) = 0;
		virtual void GotInteger(const char* name, size_t name_len, int value) = 0;
		virtual void GotBoolean(const char* name, size_t name_len, bool value) = 0;
		virtual void GotNull(const char* name, size_t name_len) = 0;
		virtual ~CallbackInterface(void) {}
	};
	
	JsonParser2(void);
	
	// file [in/out]: Pointer to an already-opened file with read access in
	//   "translate" mode.
	// data_buffer [in/out]: If NULL, one will be allocated and freed
	//   automatically.  Its size will be the system page size.
	// fread_buffer_size_in_elements [in]: Number of elements in the given
	//   buffer for reading in data.
	// result [in/out]: Will hold the file starting at where it points.
	//   If the DataMapMutator was already pointing at some place inside a file,
	//   the file will be parsed into that location.
	// RETURN: true on success, false on failure.
	bool ParseEntireFile(FILE* file,
	                     char* fread_buffer,
	                     size_t fread_buffer_size_in_elements,
	                     CallbackInterface* data_callback);
    
    bool ParseEntireFile(const char* filepath,
                         char* fread_buffer,
                         size_t fread_buffer_size_in_elements,
                         CallbackInterface* data_callback);
    bool ParseEntireFile(const WCHAR* filepath,
                         char* fread_buffer,
                         size_t fread_buffer_size_in_elements,
                         CallbackInterface* data_callback);
	
	// PRE: If beginning on a new set of data, you must Reset() this first.
	bool ParseBuffer(const char* buffer, size_t buffer_size,
	                 CallbackInterface* data_callback);
	
	// Use Reset before you parse different data.  Such as if you want to parse
	//   a totally different set of data; after a successful, failed, or
	//   (user-)canceled parse.
	void Reset(void);
	
	inline ErrorStatus::Enum GetErrorCode(void) const
			{ return m_error_status; }
 
 private:
	/*
	struct ParsingStates {
		enum Enum {
			kNone = 0,
			kParsingEntireFile
		};
	 private:
		ParsingStates();
	};
	//*/
	
	void NotifyOfError(const char* message);
	
	bool IsWhitespace(char c, bool newlines_count) const;
	
	void SkipWhitespace(bool also_skip_newlines);
	
	//
	// Parser worker functions
	//
	// Handle changes in state, temporary internal copies of data,
	//    and callbacks to the user.
	//
	
	void BeginObject(void);
	void EndObject(void);
	void BeginArray(void);
	void EndArray(void);
	
	void BeginName(void);
	void ContinueName(void);
	void FinishName(void);
	
	// used by both name and data strings
	void HandleEscapedCharacter(void);
	
	void BeginStringValue(void);
	void ContinueStringValue(void);
	void FinishStringValue(void);
	
	void BeginNumberValue_AtLeadingNegative(void);
	void BeginNumberValue_AtLeadingZero(void);
	void BeginNumberValue_AtNormalDigit(void);
	void ContinueNumberValue_AfterLeadingNegative(void);
	void ContinueNumberValue_AfterLeadingZero(void);
	void ContinueNumberValue_ReadingWholeDigits(void);
	void ContinueNumberValue_ReadingFractionalDigits(void);
	void FinishNumberValueZero(void);
	void FinishNumberValueIntegral(void);
	void FinishNumberValueWithFractional(void);
	
	void BeginTrueValue(void);
	void ContinueTrueValue(void);
	void FinishTrueValue(void);
	void BeginFalseValue(void);
	void ContinueFalseValue(void);
	void FinishFalseValue(void);
	
	void BeginNullValue(void);
	void ContinueNullValue(void);
	void FinishNullValue(void);
	
	// only called after the first value in an object/array.
	void ClearNameAndDataBuffers(void);
	
	//
	// Data
	//
	
	// all just for reading in from a file
	char m_temp_name[kMaxNameLength + 1];
	char m_temp_data[kMaxStringLength + 1];
	// always points at one-past-the-last element
	size_t m_temp_name_index;
	size_t m_temp_data_index;
	
	// holds true for objects, false for arrays.  Needed to keep proper track
	//   of what data has names, and what doesn't.
	bool m_object_type_stack[kMaxDepth];
	// points to one-past-the-last element we're using.
	size_t m_object_type_stack_index;
	
	ErrorStatus::Enum m_error_status;
	ParserStatus::Enum m_parser_status;
	
	CallbackInterface* m_data_callback;
	
	size_t m_current_row;
	size_t m_current_column;
	
	// parse-in-progress data
	const char* m_source;
	size_t m_source_size;
	size_t m_source_index;
};

} // namespace Core
