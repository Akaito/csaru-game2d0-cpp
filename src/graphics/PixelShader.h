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

class PixelShader : public IPixelShader {

private: // Data

    std::wstring        m_name;
    ID3D11PixelShader * m_shader;
    
public: // Construction

    PixelShader ();
    virtual ~PixelShader ();
    
public: // Queries

    const std::wstring & GetName ()   { return m_name; }
    ID3D11PixelShader *  GetShader () { return m_shader; }

public: // Commands

    bool Compile (
        const std::wstring & name,
        const std::wstring & filepath,
        const std::string &  entryFunction
    );

};
