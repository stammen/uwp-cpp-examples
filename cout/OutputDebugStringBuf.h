#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <ostream>
#include <vector>
#include <Windows.h>

// From http://stackoverflow.com/questions/5248704/how-to-redirect-stdout-to-output-window-from-visual-studio

/// \brief This class is a derivate of basic_stringbuf which will output all the written data using the OutputDebugString function
template<typename TChar, typename TTraits = std::char_traits<TChar>>
class OutputDebugStringBuf : public std::basic_stringbuf<TChar, TTraits> 
{
public:
    OutputDebugStringBuf() : _buffer(256)
    {
        setg(nullptr, nullptr, nullptr);
        setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
    }
    
    virtual ~OutputDebugStringBuf()
    {

    }

protected:
    static_assert(std::is_same<TChar, char>::value || std::is_same<TChar, wchar_t>::value, "OutputDebugStringBuf only supports char and wchar_t types");

    virtual int sync() override
    {
        try
        {
            MessageOutputer<TChar, TTraits>()(pbase(), pptr());
            setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
            return 0;
        }
        catch (...)
        {
	        return -1;
        }
    }

    virtual int_type overflow(int_type c = TTraits::eof()) override
    {
        auto syncRet = sync();
        if (c != TTraits::eof()) 
        {
            _buffer[0] = c;
            setp(_buffer.data(), _buffer.data() + 1, _buffer.data() + _buffer.size());
        }
        return syncRet == -1 ? TTraits::eof() : 0;
    }

private:
    std::vector<TChar> _buffer;

    template<typename TChar, typename TTraits>
    struct MessageOutputer;

    template<>
    struct MessageOutputer<char, std::char_traits<char>> 
    {
        template<typename TIterator>
        void operator()(TIterator begin, TIterator end) const 
        {
            std::string s(begin, end);
            OutputDebugStringA(s.c_str());
        }
    };

    template<>
    struct MessageOutputer<wchar_t, std::char_traits<wchar_t>> 
    {
        template<typename TIterator>
        void operator()(TIterator begin, TIterator end) const 
        {
            std::wstring s(begin, end);
            OutputDebugStringW(s.c_str());
        }
    };
};

class OutputDebugStringBufA : OutputDebugStringBuf<char>
{
public:
    OutputDebugStringBufA()
    {
        // save the previous rdbuf
        m_rdbuf = std::cout.rdbuf();
        std::cout.rdbuf(this);
    }

    virtual ~OutputDebugStringBufA()
    {
        // restore the previous rdbuf
        std::cout.rdbuf(m_rdbuf);
    }

private:
    std::basic_streambuf<char, std::char_traits<char>>* m_rdbuf;
};


class OutputDebugStringBufW : OutputDebugStringBuf<wchar_t>
{
public:
    OutputDebugStringBufW()
    {
        // save the previous rdbuf
        m_rdbuf = std::wcout.rdbuf();
        std::wcout.rdbuf(this);
    }

    virtual ~OutputDebugStringBufW()
    {
        // restore the previous rdbuf
        std::wcout.rdbuf(m_rdbuf);
    }

private:
    std::basic_streambuf<wchar_t, std::char_traits<wchar_t>>* m_rdbuf;
};
