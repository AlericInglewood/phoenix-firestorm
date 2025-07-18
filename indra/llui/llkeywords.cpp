/**
 * @file llkeywords.cpp
 * @brief Keyword list for LSL
 *
 * $LicenseInfo:firstyear=2000&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "linden_common.h"

#include <iostream>
#include <fstream>

#include "llkeywords.h"
#include "llsdserialize.h"
#include "lltexteditor.h"
#include "llstl.h"

inline bool LLKeywordToken::isHead(const llwchar* s) const
{
    // strncmp is much faster than string compare
    bool res = true;
    const llwchar* t = mToken.c_str();
    auto len = mToken.size();
    for (S32 i=0; i<len; i++)
    {
        if (s[i] != t[i])
        {
            res = false;
            break;
        }
    }
    return res;
}

inline bool LLKeywordToken::isTail(const llwchar* s) const
{
    bool res = true;
    const llwchar* t = mDelimiter.c_str();
    auto len = mDelimiter.size();
    for (S32 i=0; i<len; i++)
    {
        if (s[i] != t[i])
        {
            res = false;
            break;
        }
    }
    return res;
}

LLKeywords::LLKeywords()
:   mLoaded(false)
{
}

LLKeywords::~LLKeywords()
{
    std::for_each(mWordTokenMap.begin(), mWordTokenMap.end(), DeletePairedPointer());
    mWordTokenMap.clear();
    std::for_each(mLineTokenList.begin(), mLineTokenList.end(), DeletePointer());
    mLineTokenList.clear();
    std::for_each(mDelimiterTokenList.begin(), mDelimiterTokenList.end(), DeletePointer());
    mDelimiterTokenList.clear();
}

// Add the token as described
void LLKeywords::addToken(LLKeywordToken::ETokenType type,
                          const std::string& key_in,
                          const LLUIColor& color,
                          const std::string& tool_tip_in,
                          const std::string& delimiter_in)
{
    std::string tip_text = tool_tip_in;
    LLStringUtil::replaceString(tip_text, "\\n", "\n" );
    LLStringUtil::replaceString(tip_text, "\t", " " );
    if (tip_text.empty())
    {
        tip_text = "[no info]";
    }
    LLWString tool_tip = utf8str_to_wstring(tip_text);

    LLWString key = utf8str_to_wstring(key_in);
    LLWString delimiter = utf8str_to_wstring(delimiter_in);
    switch(type)
    {
    case LLKeywordToken::TT_CONSTANT:
    case LLKeywordToken::TT_CONTROL:
    case LLKeywordToken::TT_EVENT:
    case LLKeywordToken::TT_FUNCTION:
    case LLKeywordToken::TT_LABEL:
    case LLKeywordToken::TT_SECTION:
    case LLKeywordToken::TT_TYPE:
    case LLKeywordToken::TT_WORD:
        mWordTokenMap[key] = new LLKeywordToken(type, color, key, tool_tip, LLWStringUtil::null);
        break;

    case LLKeywordToken::TT_LINE:
        mLineTokenList.push_front(new LLKeywordToken(type, color, key, tool_tip, LLWStringUtil::null));
        break;

    case LLKeywordToken::TT_TWO_SIDED_DELIMITER:
    case LLKeywordToken::TT_DOUBLE_QUOTATION_MARKS:
    case LLKeywordToken::TT_ONE_SIDED_DELIMITER:
        mDelimiterTokenList.push_front(new LLKeywordToken(type, color, key, tool_tip, delimiter));
        break;

    default:
        llassert(0);
    }
}

std::string LLKeywords::getArguments(LLSD& arguments)
{
    std::string argString = "";

    if (arguments.isArray())
    {
        auto argsCount = arguments.size();
        LLSD::array_iterator arrayIt = arguments.beginArray();
        for ( ; arrayIt != arguments.endArray(); ++arrayIt)
        {
            LLSD& args = (*arrayIt);
            if (args.isMap())
            {
                LLSD::map_iterator argsIt = args.beginMap();
                for ( ; argsIt != args.endMap(); ++argsIt)
                {
                    argString += argsIt->second.get("type").asString() + " " + argsIt->first;
                    if (argsCount-- > 1)
                    {
                        argString += ", ";
                    }
                }
            }
            else
            {
                LL_WARNS("SyntaxLSL") << "Argument array contains a non-map element!" << LL_ENDL;
            }
        }
    }
    else if (!arguments.isUndefined())
    {
        LL_WARNS("SyntaxLSL") << "Not an array! Invalid arguments LLSD passed to function." << arguments << LL_ENDL;
    }
    return argString;
}

std::string LLKeywords::getAttribute(std::string_view key)
{
    attribute_iterator_t it = mAttributes.find(key);
    return (it != mAttributes.end()) ? it->second : "";
}

LLUIColor LLKeywords::getColorGroup(std::string_view key_in)
{
    std::string color_group = "ScriptText";
    if (key_in == "functions")
    {
        color_group = "SyntaxLslFunction";
    }
    else if (key_in == "controls")
    {
        color_group = "SyntaxLslControlFlow";
    }
    else if (key_in == "events")
    {
        color_group = "SyntaxLslEvent";
    }
    else if (key_in == "types")
    {
        color_group = "SyntaxLslDataType";
    }
    else if (key_in == "misc-flow-label")
    {
        color_group = "SyntaxLslControlFlow";
    }
    else if (key_in =="deprecated")
    {
        color_group = "SyntaxLslDeprecated";
    }
    else if (key_in =="god-mode")
    {
        color_group = "SyntaxLslGodMode";
    }
    // <FS:Ansariel> Split constant types up
    //else if (key_in == "constants"
    //       || key_in == "constants-integer"
    //       || key_in == "constants-float"
    //       || key_in == "constants-string"
    //       || key_in == "constants-key"
    //       || key_in == "constants-rotation"
    //       || key_in == "constants-vector")
    else if (key_in == "constants")
    // </FS:Ansariel>
    {
        color_group = "SyntaxLslConstant";
    }
    // <FS:Ansariel> Split constant types up
    else if (key_in == "constants-integer")
    {
        color_group = "SyntaxLslIntegerConstant";
    }
    else if (key_in == "constants-float")
    {
        color_group = "SyntaxLslFloatConstant";
    }
    else if (key_in == "constants-string"
             || key_in == "constants-key")
    {
        color_group = "SyntaxLslStringConstant";
    }
    else if (key_in == "constants-rotation"
             || key_in == "constants-vector")
    {
        color_group = "SyntaxLslCompoundConstant";
    }
    // </FS:Ansariel>
    else
    {
        LL_WARNS("SyntaxLSL") << "Color key '" << key_in << "' not recognized." << LL_ENDL;
    }

    return LLUIColorTable::instance().getColor(color_group);
}

void LLKeywords::initialize(LLSD SyntaxXML)
{
    mSyntax = SyntaxXML;
    mLoaded = true;
}

void LLKeywords::processTokens()
{
    if (!mLoaded)
    {
        return;
    }

    // Add 'standard' stuff: Quotes, Comments, Strings, Labels, etc. before processing the LLSD
    std::string delimiter;
    addToken(LLKeywordToken::TT_LABEL, "@", getColorGroup("misc-flow-label"), "Label\nTarget for jump statement", delimiter );
    addToken(LLKeywordToken::TT_ONE_SIDED_DELIMITER, "//", LLUIColorTable::instance().getColor("SyntaxLslComment"), "Comment (single-line)\nNon-functional commentary or disabled code", delimiter );
    addToken(LLKeywordToken::TT_TWO_SIDED_DELIMITER, "/*", LLUIColorTable::instance().getColor("SyntaxLslComment"), "Comment (multi-line)\nNon-functional commentary or disabled code", "*/" );
    addToken(LLKeywordToken::TT_DOUBLE_QUOTATION_MARKS, "\"", LLUIColorTable::instance().getColor("SyntaxLslStringLiteral"), "String literal", "\"" );

    LLSD::map_iterator itr = mSyntax.beginMap();
    for ( ; itr != mSyntax.endMap(); ++itr)
    {
        if (itr->first == "llsd-lsl-syntax-version")
        {
            // Skip over version key.
        }
        else
        {
            if (itr->second.isMap())
            {
                processTokensGroup(itr->second, itr->first);
            }
            else
            {
                LL_WARNS("LSL-Tokens-Processing") << "Map for " + itr->first + " entries is missing! Ignoring." << LL_ENDL;
            }
        }
    }
    LL_INFOS("SyntaxLSL") << "Finished processing tokens." << LL_ENDL;
}

void LLKeywords::processTokensGroup(const LLSD& tokens, std::string_view group)
{
    LLUIColor color;
    LLUIColor color_group;
    LLUIColor color_deprecated = getColorGroup("deprecated");
    LLUIColor color_god_mode = getColorGroup("god-mode");

    LLKeywordToken::ETokenType token_type = LLKeywordToken::TT_UNKNOWN;
    // If a new token type is added here, it must also be added to the 'addToken' method
    if (group == "constants")
    {
        token_type = LLKeywordToken::TT_CONSTANT;
    }
    else if (group == "controls")
    {
        token_type = LLKeywordToken::TT_CONTROL;
    }
    else if (group == "events")
    {
        token_type = LLKeywordToken::TT_EVENT;
    }
    else if (group == "functions")
    {
        token_type = LLKeywordToken::TT_FUNCTION;
    }
    else if (group == "label")
    {
        token_type = LLKeywordToken::TT_LABEL;
    }
    else if (group == "types")
    {
        token_type = LLKeywordToken::TT_TYPE;
    }

    color_group = getColorGroup(group);
    LL_DEBUGS("SyntaxLSL") << "Group: '" << group << "', using color: '" << color_group.get() << "'" << LL_ENDL;

    if (tokens.isMap())
    {
        LLSD::map_const_iterator outer_itr = tokens.beginMap();
        for ( ; outer_itr != tokens.endMap(); ++outer_itr )
        {
            if (outer_itr->second.isMap())
            {
                mAttributes.clear();
                LLSD arguments = LLSD();
                LLSD::map_const_iterator inner_itr = outer_itr->second.beginMap();
                for ( ; inner_itr != outer_itr->second.endMap(); ++inner_itr )
                {
                    if (inner_itr->first == "arguments")
                    {
                        if (inner_itr->second.isArray())
                        {
                            arguments = inner_itr->second;
                        }
                    }
                    else if (!inner_itr->second.isMap() && !inner_itr->second.isArray())
                    {
                        mAttributes[inner_itr->first] = inner_itr->second.asString();
                    }
                    else
                    {
                        LL_WARNS("SyntaxLSL") << "Not a valid attribute: " << inner_itr->first << LL_ENDL;
                    }
                }

                std::string tooltip = "";
                switch (token_type)
                {
                    case LLKeywordToken::TT_CONSTANT:
                        if (getAttribute("type").length() > 0)
                        {
                            color_group = getColorGroup(std::string(group) + "-" + getAttribute("type"));
                        }
                        else
                        {
                            color_group = getColorGroup(group);
                        }
                        tooltip = "Type: " + getAttribute("type") + ", Value: " + getAttribute("value");
                        break;
                    case LLKeywordToken::TT_EVENT:
                        tooltip = outer_itr->first + "(" + getArguments(arguments) + ")";
                        break;
                    case LLKeywordToken::TT_FUNCTION:
                        tooltip = getAttribute("return") + " " + outer_itr->first + "(" + getArguments(arguments) + ");";
                        tooltip.append("\nEnergy: ");
                        tooltip.append(getAttribute("energy").empty() ? "0.0" : getAttribute("energy"));
                        if (!getAttribute("sleep").empty())
                        {
                            tooltip += ", Sleep: " + getAttribute("sleep");
                        }
                    default:
                        break;
                }

                if (!getAttribute("tooltip").empty())
                {
                    if (!tooltip.empty())
                    {
                        tooltip.append("\n");
                    }
                    tooltip.append(getAttribute("tooltip"));
                }

                color = getAttribute("deprecated") == "true" ? color_deprecated : color_group;

                if (getAttribute("god-mode") == "true")
                {
                    color = color_god_mode;
                }

                addToken(token_type, outer_itr->first, color, tooltip);
            }
        }
    }
    else if (tokens.isArray())  // Currently nothing should need this, but it's here for completeness
    {
        LL_INFOS("SyntaxLSL") << "Curious, shouldn't be an array here; adding all using color " << color.get() << LL_ENDL;
        for (S32 count = 0; count < tokens.size(); ++count)
        {
            addToken(token_type, tokens[count], color, "");
        }
    }
    else
    {
        LL_WARNS("SyntaxLSL") << "Invalid map/array passed: '" << tokens << "'" << LL_ENDL;
    }
}

LLKeywords::WStringMapIndex::WStringMapIndex(const WStringMapIndex& other)
{
    if(other.mOwner)
    {
        copyData(other.mData, other.mLength);
    }
    else
    {
        mOwner = false;
        mLength = other.mLength;
        mData = other.mData;
    }
}

LLKeywords::WStringMapIndex::WStringMapIndex(const LLWString& str)
{
    copyData(str.data(), str.size());
}

LLKeywords::WStringMapIndex::WStringMapIndex(const llwchar *start, size_t length)
:   mData(start)
,   mLength(length)
,   mOwner(false)
{
}

LLKeywords::WStringMapIndex::~WStringMapIndex()
{
    if (mOwner)
    {
        delete[] mData;
    }
}

void LLKeywords::WStringMapIndex::copyData(const llwchar *start, size_t length)
{
    llwchar *data = new llwchar[length];
    memcpy((void*)data, (const void*)start, length * sizeof(llwchar));

    mOwner = true;
    mLength = length;
    mData = data;
}

bool LLKeywords::WStringMapIndex::operator<(const LLKeywords::WStringMapIndex &other) const
{
    // NOTE: Since this is only used to organize a std::map, it doesn't matter if it uses correct collate order or not.
    // The comparison only needs to strictly order all possible strings, and be stable.

    bool result = false;
    const llwchar* self_iter = mData;
    const llwchar* self_end = mData + mLength;
    const llwchar* other_iter = other.mData;
    const llwchar* other_end = other.mData + other.mLength;

    while(true)
    {
        if(other_iter >= other_end)
        {
            // We've hit the end of other.
            // This covers two cases: other being shorter than self, or the strings being equal.
            // In either case, we want to return false.
            result = false;
            break;
        }
        else if(self_iter >= self_end)
        {
            // self is shorter than other.
            result = true;
            break;
        }
        else if(*self_iter != *other_iter)
        {
            // The current character differs.  The strings are not equal.
            result = *self_iter < *other_iter;
            break;
        }

        self_iter++;
        other_iter++;
    }

    return result;
}

LLTrace::BlockTimerStatHandle FTM_SYNTAX_COLORING("Syntax Coloring");

// Walk through a string, applying the rules specified by the keyword token list and
// create a list of color segments.
void LLKeywords::findSegments(std::vector<LLTextSegmentPtr>* seg_list, const LLWString& wtext, LLTextEditor& editor, LLStyleConstSP style)
{
    LL_RECORD_BLOCK_TIME(FTM_SYNTAX_COLORING);
    seg_list->clear();

    if( wtext.empty() )
    {
        return;
    }

    S32 text_len = static_cast<S32>(wtext.size()) + 1;

    // <FS:Ansariel> Script editor ignoring font selection
    //seg_list->push_back( new LLNormalTextSegment( style, 0, text_len, editor ) );
    LLStyleSP actual_style = getDefaultStyle(editor);
    actual_style->setColor(style->getColor());
    seg_list->push_back( new LLNormalTextSegment( actual_style, 0, text_len, editor ) );
    // </FS:Ansariel>

    const llwchar* base = wtext.c_str();
    const llwchar* cur = base;
    while( *cur )
    {
        if( *cur == '\n' || cur == base )
        {
            if( *cur == '\n' )
            {
                // <FS:Ansariel> Script editor ignoring font selection
                //LLTextSegmentPtr text_segment = new LLLineBreakTextSegment(style, (S32)(cur - base));
                LLTextSegmentPtr text_segment = new LLLineBreakTextSegment(getDefaultStyle(editor), (S32)(cur - base));
                // </FS:Ansariel>
                text_segment->setToken( 0 );
                insertSegment( *seg_list, text_segment, text_len, style, editor);
                cur++;
                if( !*cur || *cur == '\n' )
                {
                    continue;
                }
            }

            // Skip white space
            while( *cur && iswspace(*cur) && (*cur != '\n')  )
            {
                cur++;
            }
            if( !*cur || *cur == '\n' )
            {
                continue;
            }

            // cur is now at the first non-whitespace character of a new line

            // Line start tokens
            {
                bool line_done = false;
                for (token_list_t::iterator iter = mLineTokenList.begin();
                     iter != mLineTokenList.end(); ++iter)
                {
                    LLKeywordToken* cur_token = *iter;
                    if( cur_token->isHead( cur ) )
                    {
                        S32 seg_start = (S32)(cur - base);
                        while( *cur && *cur != '\n' )
                        {
                            // skip the rest of the line
                            cur++;
                        }
                        S32 seg_end = (S32)(cur - base);

                        //create segments from seg_start to seg_end
                        insertSegments(wtext, *seg_list,cur_token, text_len, seg_start, seg_end, style, editor);
                        line_done = true; // to break out of second loop.
                        break;
                    }
                }

                if( line_done )
                {
                    continue;
                }
            }
        }

        // Skip white space
        while( *cur && iswspace(*cur) && (*cur != '\n')  )
        {
            cur++;
        }

        while( *cur && *cur != '\n' )
        {
            // Check against delimiters
            {
                S32 seg_start = 0;
                LLKeywordToken* cur_delimiter = NULL;
                for (token_list_t::iterator iter = mDelimiterTokenList.begin();
                     iter != mDelimiterTokenList.end(); ++iter)
                {
                    LLKeywordToken* delimiter = *iter;
                    if( delimiter->isHead( cur ) )
                    {
                        cur_delimiter = delimiter;
                        break;
                    }
                }

                if( cur_delimiter )
                {
                    S32 between_delimiters = 0;
                    S32 seg_end = 0;

                    seg_start = (S32)(cur - base);
                    cur += cur_delimiter->getLengthHead();

                    LLKeywordToken::ETokenType type = cur_delimiter->getType();
                    if( type == LLKeywordToken::TT_TWO_SIDED_DELIMITER || type == LLKeywordToken::TT_DOUBLE_QUOTATION_MARKS )
                    {
                        while( *cur && !cur_delimiter->isTail(cur))
                        {
                            // Check for an escape sequence.
                            if (type == LLKeywordToken::TT_DOUBLE_QUOTATION_MARKS && *cur == '\\')
                            {
                                // Count the number of backslashes.
                                S32 num_backslashes = 0;
                                while (*cur == '\\')
                                {
                                    num_backslashes++;
                                    between_delimiters++;
                                    cur++;
                                }
                                // If the next character is the end delimiter?
                                if (cur_delimiter->isTail(cur))
                                {
                                    // If there was an odd number of backslashes, then this delimiter
                                    // does not end the sequence.
                                    if (num_backslashes % 2 == 1)
                                    {
                                        between_delimiters++;
                                        cur++;
                                    }
                                    else
                                    {
                                        // This is an end delimiter.
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                between_delimiters++;
                                cur++;
                            }
                        }

                        if( *cur )
                        {
                            cur += cur_delimiter->getLengthHead();
                            seg_end = seg_start + between_delimiters + cur_delimiter->getLengthHead() + cur_delimiter->getLengthTail();
                        }
                        else
                        {
                            // eof
                            seg_end = seg_start + between_delimiters + cur_delimiter->getLengthHead();
                        }
                    }
                    else
                    {
                        llassert( cur_delimiter->getType() == LLKeywordToken::TT_ONE_SIDED_DELIMITER );
                        // Left side is the delimiter.  Right side is eol or eof.
                        while( *cur && ('\n' != *cur) )
                        {
                            between_delimiters++;
                            cur++;
                        }
                        seg_end = seg_start + between_delimiters + cur_delimiter->getLengthHead();
                    }

                    insertSegments(wtext, *seg_list,cur_delimiter, text_len, seg_start, seg_end, style, editor);
                    /*
                    // <FS:Ansariel> Script editor ignoring font selection
                    //LLTextSegmentPtr text_segment = new LLNormalTextSegment( cur_delimiter->getColor(), seg_start, seg_end, editor );
                    LLStyleSP seg_style = getDefaultStyle(editor);
                    seg_style->setColor(defaultColor);
                    LLTextSegmentPtr text_segment = new LLNormalTextSegment( seg_style, seg_start, seg_end, editor );
                    // </FS:Ansariel>

                    text_segment->setToken( cur_delimiter );
                    insertSegment( seg_list, text_segment, text_len, defaultColor, editor);
                    */
                    // Note: we don't increment cur, since the end of one delimited seg may be immediately
                    // followed by the start of another one.
                    continue;
                }
            }

            // check against words
            llwchar prev = cur > base ? *(cur-1) : 0;
            // NaCl - LSL Preprocessor
            //if( !iswalnum( prev ) && (prev != '_') )
            if( !iswalnum( prev ) && (prev != '_') && (prev != '#'))
            {
                const llwchar* p = cur;
                //while( iswalnum( *p ) || (*p == '_') )
                while( *p && ( iswalnum( *p ) || (*p == '_') || (*p == '#') ) )
                // NaCl End
                {
                    p++;
                }
                S32 seg_len = (S32)(p - cur);
                if( seg_len > 0 )
                {
                    WStringMapIndex word( cur, seg_len );
                    word_token_map_t::iterator map_iter = mWordTokenMap.find(word);
                    if( map_iter != mWordTokenMap.end() )
                    {
                        LLKeywordToken* cur_token = map_iter->second;
                        S32 seg_start = (S32)(cur - base);
                        S32 seg_end = seg_start + seg_len;

                        // LL_INFOS("SyntaxLSL") << "Seg: [" << word.c_str() << "]" << LL_ENDL;

                        insertSegments(wtext, *seg_list,cur_token, text_len, seg_start, seg_end, style, editor);
                    }
                    cur += seg_len;
                    continue;
                }
            }

            if( *cur && *cur != '\n' )
            {
                cur++;
            }
        }
    }
}

void LLKeywords::insertSegments(const LLWString& wtext, std::vector<LLTextSegmentPtr>& seg_list, LLKeywordToken* cur_token, S32 text_len, S32 seg_start, S32 seg_end, LLStyleConstSP style, LLTextEditor& editor )
{
    std::string::size_type pos = wtext.find('\n',seg_start);

    // <FS:Ansariel> Script editor ignoring font selection
    //LLStyleConstSP cur_token_style = new LLStyle(LLStyle::Params().font(style->getFont()).color(cur_token->getColor()));

    while (pos!=-1 && pos < (std::string::size_type)seg_end)
    {
        if (pos!=seg_start)
        {
            // <FS:Ansariel> Script editor ignoring font selection
            //LLTextSegmentPtr text_segment = new LLNormalTextSegment( cur_token->getColor(), seg_start, static_cast<S32>(pos), editor );
            LLStyleSP style = getDefaultStyle(editor);
            style->setColor(cur_token->getColor());
            LLTextSegmentPtr text_segment = new LLNormalTextSegment( style, seg_start, static_cast<S32>(pos), editor );
            // </FS:Ansariel>
            text_segment->setToken( cur_token );
            insertSegment( seg_list, text_segment, text_len, style, editor);
        }

        // <FS:Ansariel> Script editor ignoring font selection
        //LLTextSegmentPtr text_segment = new LLLineBreakTextSegment(style, static_cast<S32>(pos));
        LLTextSegmentPtr text_segment = new LLLineBreakTextSegment(getDefaultStyle(editor), static_cast<S32>(pos));
        // </FS:Ansariel>
        text_segment->setToken( cur_token );
        insertSegment( seg_list, text_segment, text_len, style, editor);

        seg_start = static_cast<S32>(pos) + 1;
        pos = wtext.find('\n',seg_start);
    }

    // <FS:Ansariel> Script editor ignoring font selection
    //LLTextSegmentPtr text_segment = new LLNormalTextSegment(cur_token_style, seg_start, seg_end, editor);
    LLStyleSP actual_style = getDefaultStyle(editor);
    actual_style->setColor(cur_token->getColor());
    LLTextSegmentPtr text_segment = new LLNormalTextSegment(actual_style, seg_start, seg_end, editor);
    // </FS:Ansariel>
    text_segment->setToken( cur_token );
    insertSegment( seg_list, text_segment, text_len, style, editor);
}

void LLKeywords::insertSegment(std::vector<LLTextSegmentPtr>& seg_list, LLTextSegmentPtr new_segment, S32 text_len, const LLUIColor& defaultColor, LLTextEditor& editor )
{
    LLTextSegmentPtr last = seg_list.back();
    S32 new_seg_end = new_segment->getEnd();

    if( new_segment->getStart() == last->getStart() )
    {
        seg_list.pop_back();
    }
    else
    {
        last->setEnd( new_segment->getStart() );
    }
    seg_list.push_back( new_segment );

    if( new_seg_end < text_len )
    {
        // <FS:Ansariel> Script editor ignoring font selection
        //seg_list.push_back( new LLNormalTextSegment( defaultColor, new_seg_end, text_len, editor ) );
        LLStyleSP style = getDefaultStyle(editor);
        style->setColor(defaultColor);
        seg_list.push_back( new LLNormalTextSegment( style, new_seg_end, text_len, editor ) );
        // </FS:Ansariel>
    }
}

void LLKeywords::insertSegment(std::vector<LLTextSegmentPtr>& seg_list, LLTextSegmentPtr new_segment, S32 text_len, LLStyleConstSP style, LLTextEditor& editor )
{
    LLTextSegmentPtr last = seg_list.back();
    S32 new_seg_end = new_segment->getEnd();

    if( new_segment->getStart() == last->getStart() )
    {
        seg_list.pop_back();
    }
    else
    {
        last->setEnd( new_segment->getStart() );
    }
    seg_list.push_back( new_segment );

    if( new_seg_end < text_len )
    {
        // <FS:Ansariel> Script editor ignoring font selection
        //seg_list.push_back( new LLNormalTextSegment( style, new_seg_end, text_len, editor ) );
        LLStyleSP actual_style = getDefaultStyle(editor);
        actual_style->setColor(style->getColor());
        seg_list.push_back(new LLNormalTextSegment(actual_style, new_seg_end, text_len, editor));
        // </FS:Ansariel>
    }
}

// <FS:Ansariel> Re-add support for Cinder's legacy file format
bool LLKeywords::loadFromLegacyFile(const std::string& filename)
{
    mLoaded = false;

    ///////////////////////////////////////////
    // Parse the script library xml

    LLXMLNodePtr xml_root;
    if ( (!LLUICtrlFactory::getLayeredXMLNode(filename, xml_root)) || (xml_root.isNull()) || (!xml_root->hasName("script_library")) )
    {
        LL_WARNS() << "Could not read the script library (" << filename << ")" << LL_ENDL;
        return false;
    }
    for (LLXMLNode* pNode = xml_root->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
    {
        if (pNode->hasName("keywords"))
        {
            std::string keyword;
            for (LLXMLNode* pStringNode = pNode->getFirstChild(); pStringNode != NULL; pStringNode = pStringNode->getNextSibling())
            {
                std::string tool_tip;
                LLColor4 cur_color(LLUIColorTable::instance().getColor("ScriptText"));
                LLKeywordToken::ETokenType cur_type = LLKeywordToken::TT_WORD;
                if (!pStringNode->getAttributeString("name", keyword))
                    continue;
                if (pStringNode->hasName("integer_constant"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslIntegerConstant");
                }
                else if (pStringNode->hasName("event"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslEvent");

                }
                else if (pStringNode->hasName("section"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslSection");

                }
                else if (pStringNode->hasName("data_type"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslDataType");

                }
                else if (pStringNode->hasName("string_constant"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslStringConstant");
                }
                else if (pStringNode->hasName("float_constant"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslFloatConstant");
                }
                else if (pStringNode->hasName("compound_constant"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslCompoundConstant");
                }
                else if (pStringNode->hasName("flow_control_keyword"))
                {
                    cur_type = LLKeywordToken::TT_WORD;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslControlFlow");
                }
                else if (pStringNode->hasName("flow_control_label"))
                {
                    cur_type = LLKeywordToken::TT_LINE;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslControlFlow");
                }
                else if (pStringNode->hasName("comment"))
                {
                    cur_type = LLKeywordToken::TT_ONE_SIDED_DELIMITER;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslComment");
                }
                else if (pStringNode->hasName("block_comment"))
                {
                    cur_type = LLKeywordToken::TT_TWO_SIDED_DELIMITER;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslComment");
                }
                else if (pStringNode->hasName("string_literal"))
                {
                    cur_type = LLKeywordToken::TT_DOUBLE_QUOTATION_MARKS;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslStringLiteral");
                }
                else if (pStringNode->hasName("preprocessor"))
                {
                    cur_type = LLKeywordToken::TT_ONE_SIDED_DELIMITER;
                    cur_color = LLUIColorTable::instance().getColor("SyntaxLslPreprocessor");
                }
                if (!pStringNode->getAttributeString("desc", tool_tip))
                {
                    tool_tip = "No Description available";
                }

                std::string delimiter;
                if (cur_type == LLKeywordToken::TT_DOUBLE_QUOTATION_MARKS)
                {
                    // Closing delimiter is identical to the opening one.
                    delimiter = keyword;
                }
                else if (cur_type == LLKeywordToken::TT_TWO_SIDED_DELIMITER)
                {
                    std::string token_buffer(keyword);
                    LLStringUtil::trim(token_buffer);

                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep_word(" ");
                    tokenizer word_tokens(token_buffer, sep_word);
                    tokenizer::iterator token_word_iter = word_tokens.begin();

                    if( !token_buffer.empty() && token_word_iter != word_tokens.end() )
                    {
                        // first word is the keyword or a left delimiter
                        keyword = (*token_word_iter);
                        LLStringUtil::trim(keyword);

                        // second word may be a right delimiter
                        while (delimiter.length() == 0 && ++token_word_iter != word_tokens.end())
                        {
                            delimiter = *token_word_iter;
                            LLStringUtil::trim(delimiter);
                        }
                    }
                }

                if( !tool_tip.empty() )
                {
                    // Replace ; with \n for multi-line tool tips.
                    LLStringUtil::replaceChar( tool_tip, ';', '\n' );
                    addToken(cur_type, keyword, cur_color, tool_tip, delimiter );
                }
                else
                {
                    addToken(cur_type, keyword, cur_color, LLStringUtil::null, delimiter );
                }
            }
        }
    }

    mLoaded = true;
    return mLoaded;
}
// </FS:Ansariel>

// <FS:Ansariel> Script editor ignoring font selection
LLStyleSP LLKeywords::getDefaultStyle(const LLTextEditor& editor)
{
    LLStyleSP style(new LLStyle(LLStyle::Params()));
    style->setFont(editor.getFont());
    return style;
}
// </FS:Ansariel>

#ifdef _DEBUG
void LLKeywords::dump()
{
    LL_INFOS() << "LLKeywords" << LL_ENDL;


    LL_INFOS() << "LLKeywords::sWordTokenMap" << LL_ENDL;
    word_token_map_t::iterator word_token_iter = mWordTokenMap.begin();
    while( word_token_iter != mWordTokenMap.end() )
    {
        LLKeywordToken* word_token = word_token_iter->second;
        word_token->dump();
        ++word_token_iter;
    }

    LL_INFOS() << "LLKeywords::sLineTokenList" << LL_ENDL;
    for (token_list_t::iterator iter = mLineTokenList.begin();
         iter != mLineTokenList.end(); ++iter)
    {
        LLKeywordToken* line_token = *iter;
        line_token->dump();
    }


    LL_INFOS() << "LLKeywords::sDelimiterTokenList" << LL_ENDL;
    for (token_list_t::iterator iter = mDelimiterTokenList.begin();
         iter != mDelimiterTokenList.end(); ++iter)
    {
        LLKeywordToken* delimiter_token = *iter;
        delimiter_token->dump();
    }
}

void LLKeywordToken::dump()
{
    LL_INFOS() << "[" <<
        mColor.get().mV[VRED] << ", " <<
        mColor.get().mV[VGREEN] << ", " <<
        mColor.get().mV[VBLUE] << "] [" <<
        wstring_to_utf8str(mToken) << "]" <<
        LL_ENDL;
}

#endif  // DEBUG
