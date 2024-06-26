#ifndef SWARMVM_ISAPARSER
#define SWARMVM_ISAPARSER

#include <iostream>
#include <cctype>
#include "../shared/nslib.h"
#include "../errors/SwarmError.h"
#include "isa_meta.h"

using namespace nslib;

namespace swarmc::ISA {

    /** Parses SVI code into ISA instruction instances. */
    class Parser : public IStringable, public IUsesConsole {
    public:
        explicit Parser(std::istream& in) : IUsesConsole(), _in(in) {}
        ~Parser() override = default;

        [[nodiscard]] std::string toString() const override {
            return "ISA::Parser<>";
        }

        /** Get a cleaned-up list of tokens, with the whitespace removed, and strings properly grouped. */
        virtual std::vector<std::string> tokenize() {
            std::string inputString(std::istreambuf_iterator<char>(_in), {});
            std::vector<std::string> tokens;
            std::string token;

            bool hasEscape = false;
            bool hasString = false;
            bool hasComment = false;
            bool hasCommentLeader = false;

            for ( std::size_t i = 0; i < inputString.length(); i += 1 ) {
                auto c = inputString.at(i);

                // Ignore tokens w/in comments
                if ( hasComment && c != '\n' ) {
                    continue;
                }

                // If we have an escaped character, always include it in the token.
                if ( hasEscape ) {
                    token += c;
                    hasEscape = false;
                    continue;
                }

                // Keep track of whether we are in a string.
                if ( c == '"' ) {
                    hasString = !hasString;
                }

                // Look for leading escape sequences.
                if ( c == '\\' ) {
                    hasEscape = true;
                    continue;
                }

                // If we get an unescaped -, it may be the start of a comment
                if ( !hasString && !hasCommentLeader && !hasComment && c == '-' ) {
                    hasCommentLeader = true;
                    continue;
                }

                // If we get a second unescaped -, begin a comment
                if ( !hasString && hasCommentLeader && !hasComment && c == '-' ) {
                    hasComment = true;
                    continue;
                }

                // If we get something that is not a - after a comment leader,
                if ( !hasString && hasCommentLeader && !hasComment && c != '-' ) {
                    hasCommentLeader = false;
                    token += '-';  // Account for the first - we skipped.
                }

                // Non-string/escaped spaces terminate tokens
                if ( !hasString && (c == ' ' || c == '\t' || c == '\n') ) {
                    if ( !token.empty() ) tokens.push_back(token);
                    token = "";
                    hasEscape = false;
                    hasCommentLeader = false;
                    hasComment = false;
                    continue;
                }

                token += c;
            }

            if ( !token.empty() ) tokens.push_back(token);
            return tokens;
        }

        /** Parse a list of instructions from the input to this parser. */
        virtual Instructions parse() {
            auto tokens = tokenize();
            return parse(tokens);
        }

        /** Parse a list of instructions from the given list of tokens. */
        virtual Instructions parse(std::vector<std::string>& tokens) {
            Instructions is;
            for ( std::size_t i = 0; i < tokens.size(); ) {
                i += parseOne(is, tokens, i);
            }
            return is;
        }

        /**
         * Parse a single instruction from the list of tokens and insert it into the `is` list
         * starting at the `startAt`-th token in `tokens`.
         * This function should return the number of tokens which were consumed from `tokens`.
         */
        virtual std::size_t parseOne(Instructions& is, std::vector<std::string>& tokens, std::size_t startAt) {
            try {
                auto token = tokens.at(startAt);
                auto leader = token.at(0);
                if ( leader == '$' ) return parseAssignment(is, tokens, startAt);
                else if (std::isalpha(leader) || leader == '.') return parseInstruction(is, tokens, startAt);
                else throw Errors::SwarmError("Error parsing SVI: invalid token `" + token + "` (expected assignment or instruction)");
            } catch (Errors::SwarmError& e) {
                std::string last;
                if ( !is.empty() ) last = " - last instruction: " + is[is.size()-1]->toString();
                throw Errors::SwarmError(e.what() + last);
            }
        }

        /** Parse an ISA::Reference which consumes a single token from the tokens, starting at the `startAt`-th token. */
        virtual ISA::Reference* parseUnaryReference(std::string const& leader, std::vector<std::string>& tokens, std::size_t startAt) {
            if ( tokens.size() < startAt+1 ) throw Errors::SwarmError("Malformed instruction `" + leader + "` (expected 1 reference, got EOF)");
            return parseReference(tokens.at(startAt));
        }

        virtual ISA::PositionAnnotation* parsePosition(const std::string& instructionLeader, std::vector<std::string>& tokens, std::size_t startAt) {
            auto file = parseUnaryReference(instructionLeader, tokens, startAt);
            if ( file->tag() != ReferenceTag::STRING ) throw Errors::SwarmError("Malformed .position annotation (expected string, got " + file->toString() + ")");

            auto line = parseUnaryReference(instructionLeader, tokens, startAt+1);
            if ( line->tag() != ReferenceTag::NUMBER ) throw Errors::SwarmError("Malformed .position annotation (expected line number, got " + line->toString() + ")");

            auto col = parseUnaryReference(instructionLeader, tokens, startAt+2);
            if ( col->tag() != ReferenceTag::NUMBER ) throw Errors::SwarmError("Malformed .position annotation (expected col number, got " + col->toString() + ")");

            return new ISA::PositionAnnotation((StringReference*) file, (NumberReference*) line, (NumberReference*) col);
        }

        /**
         * Parse a single instruction from the token stream and append it to `is`, starting at the `startAt`-th token.
         * Returns the number of tokens which were consumed from `tokens`.
         */
        virtual std::size_t parseInstruction(Instructions& is, std::vector<std::string>& tokens, std::size_t startAt) {
            std::size_t i = 1;
            auto instructionLeader = tokens.at(startAt);

            if ( instructionLeader == ".position" ) {
                is.push_back(parsePosition(instructionLeader, tokens, startAt+i));
                i += 3;
            } else if ( instructionLeader == "beginfn" ) {
                auto lr = parseLocationReference(instructionLeader, tokens, startAt+i);
                is.push_back(new ISA::BeginFunction(
                    lr->name(),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                delete lr;
                i += 2;
            } else if ( instructionLeader == "fnparam" ) {
                is.push_back(new ISA::FunctionParam(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseLocationReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "return" ) {
                if ( countOperands(tokens, startAt+i) < 1 ) {
                    is.push_back(new ISA::Return0());
                } else {
                    is.push_back(new ISA::Return1(
                        parseUnaryReference(instructionLeader, tokens, startAt+i)
                    ));
                    i += 1;
                }
            } else if ( instructionLeader == "curry" ) {
                is.push_back(new ISA::Curry(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "call" ) {
                if ( countOperands(tokens, startAt+i) < 2 ) {
                    is.push_back(new ISA::Call0(
                        parseUnaryReference(instructionLeader, tokens, startAt+i)
                    ));
                    i += 1;
                } else {
                    is.push_back(new ISA::Call1(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                }
            } else if ( instructionLeader == "callif" ) {
                if ( countOperands(tokens, startAt+i) < 3 ) {
                    is.push_back(new ISA::CallIf0(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                } else {
                    is.push_back(new ISA::CallIf1(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                    ));
                    i += 3;
                }
            } else if ( instructionLeader == "callelse" ) {
                if ( countOperands(tokens, startAt+i) < 3 ) {
                    is.push_back(new ISA::CallElse0(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                } else {
                    is.push_back(new ISA::CallElse1(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                    ));
                    i += 3;
                }
            } else if ( instructionLeader == "pushcall" ) {
                if ( countOperands(tokens, startAt+i) < 2 ) {
                    is.push_back(new ISA::PushCall0(
                            parseUnaryReference(instructionLeader, tokens, startAt+i)
                    ));
                    i += 1;
                } else {
                    is.push_back(new ISA::PushCall1(
                            parseUnaryReference(instructionLeader, tokens, startAt+i),
                            parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                }
            } else if ( instructionLeader == "pushcallif" ) {
                if ( countOperands(tokens, startAt+i) < 3 ) {
                    is.push_back(new ISA::PushCallIf0(
                            parseUnaryReference(instructionLeader, tokens, startAt+i),
                            parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                } else {
                    is.push_back(new ISA::PushCallIf1(
                            parseUnaryReference(instructionLeader, tokens, startAt+i),
                            parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                            parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                    ));
                    i += 3;
                }
            } else if ( instructionLeader == "pushcallelse" ) {
                if ( countOperands(tokens, startAt+i) < 3 ) {
                    is.push_back(new ISA::PushCallElse0(
                            parseUnaryReference(instructionLeader, tokens, startAt+i),
                            parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                } else {
                    is.push_back(new ISA::PushCallElse1(
                            parseUnaryReference(instructionLeader, tokens, startAt+i),
                            parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                            parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                    ));
                    i += 3;
                }
            } else if ( instructionLeader == "drain" ) {
                is.push_back(new ISA::Drain());
            } else if ( instructionLeader == "retmaphas" ) {
                is.push_back(new ISA::RetMapHas(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "retmapget" ) {
                is.push_back(new ISA::RetMapGet(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "entercontext" ) {
                is.push_back(new ISA::EnterContext());
            } else if ( instructionLeader == "resumecontext" ) {
                is.push_back(new ISA::ResumeContext(
                        parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "popcontext" ) {
                is.push_back(new ISA::PopContext());
            } else if ( instructionLeader == "exit" ) {
                is.push_back(new ISA::Exit());
            } else if ( instructionLeader == "out" ) {
                is.push_back(new ISA::Out(parseUnaryReference(instructionLeader, tokens, startAt+i)));
                i += 1;
            } else if ( instructionLeader == "err" ) {
                is.push_back(new ISA::Err(parseUnaryReference(instructionLeader, tokens, startAt+i)));
                i += 1;
            } else if ( instructionLeader == "streaminit" ) {
                is.push_back(new ISA::StreamInit(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "streampush" ) {
                is.push_back(new ISA::StreamPush(
                    parseLocationReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "streampop" ) {
                is.push_back(new ISA::StreamPop(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "streamclose" ) {
                is.push_back(new ISA::StreamClose(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "streamempty" ) {
                is.push_back(new ISA::StreamEmpty(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "typify" ) {
                is.push_back(new ISA::Typify(
                    parseLocationReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "lock" ) {
                is.push_back(new ISA::Lock(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "unlock" ) {
                is.push_back(new ISA::Unlock(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "equal" ) {
                is.push_back(new ISA::IsEqual(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "scopeof" ) {
                is.push_back(new ISA::ScopeOf(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "typeof" ) {
                is.push_back(new ISA::TypeOf(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "compatible" ) {
                is.push_back(new ISA::IsCompatible(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "and" ) {
                is.push_back(new ISA::And(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "or" ) {
                is.push_back(new ISA::Or(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "xor" ) {
                is.push_back(new ISA::Xor(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "nand" ) {
                is.push_back(new ISA::Nand(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "nor" ) {
                is.push_back(new ISA::Nor(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "not" ) {
                is.push_back(new ISA::Not(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "mapinit" ) {
                is.push_back(new ISA::MapInit(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "mapset" ) {
                is.push_back(new ISA::MapSet(
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+2),
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 3;
            } else if ( instructionLeader == "mapget" ) {
                is.push_back(new ISA::MapGet(
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 2;
            } else if ( instructionLeader == "maplength" ) {
                is.push_back(new ISA::MapLength(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "mapkeys" ) {
                is.push_back(new ISA::MapKeys(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "enuminit" ) {
                is.push_back(new ISA::EnumInit(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "enumappend" ) {
                is.push_back(new ISA::EnumAppend(
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 2;
            } else if ( instructionLeader == "enumprepend" ) {
                is.push_back(new ISA::EnumPrepend(
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 2;
            } else if ( instructionLeader == "enumlength" ) {
                is.push_back(new ISA::EnumLength(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "enumget" ) {
                is.push_back(new ISA::EnumGet(
                    parseLocationReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "enumset" ) {
                is.push_back(new ISA::EnumSet(
                    parseLocationReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                ));
                i += 3;
            } else if ( instructionLeader == "enumconcat" ) {
                is.push_back(new ISA::EnumConcat(
                    parseLocationReference(instructionLeader, tokens, startAt+i),
                    parseLocationReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "enumerate" ) {
                is.push_back(new ISA::Enumerate(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseLocationReference(instructionLeader, tokens, startAt+i+1),
                    parseLocationReference(instructionLeader, tokens, startAt+i+2)
                ));
                i += 3;
            } else if ( instructionLeader == "strconcat" ) {
                is.push_back(new ISA::StringConcat(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "strlength" ) {
                is.push_back(new ISA::StringLength(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "strslice" ) {
                if ( countOperands(tokens, startAt+i) < 3 ) {
                    is.push_back(new ISA::StringSliceFrom(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                } else {
                    is.push_back(new ISA::StringSliceFromTo(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                    ));
                    i += 3;
                }
            } else if ( instructionLeader == "plus" ) {
                is.push_back(new ISA::Plus(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "minus" ) {
                is.push_back(new ISA::Minus(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "times" ) {
                is.push_back(new ISA::Times(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "divide" ) {
                is.push_back(new ISA::Divide(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "power" ) {
                is.push_back(new ISA::Power(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "mod" ) {
                is.push_back(new ISA::Mod(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "neg" ) {
                is.push_back(new ISA::Negative(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "gt" ) {
                is.push_back(new ISA::GreaterThan(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "gte" ) {
                is.push_back(new ISA::GreaterThanOrEqual(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "lt" ) {
                is.push_back(new ISA::LessThan(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "lte" ) {
                is.push_back(new ISA::LessThanOrEqual(
                        parseUnaryReference(instructionLeader, tokens, startAt+i),
                        parseUnaryReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "while" ) {
                is.push_back(new ISA::While(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseLocationReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "with" ) {
                is.push_back(new ISA::With(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseLocationReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "pushexhandler" ) {
                if ( countOperands(tokens, startAt+i) < 2 ) {
                    is.push_back(new ISA::PushExceptionHandler1(
                        parseLocationReference(instructionLeader, tokens, startAt+i)
                    ));
                    i += 1;
                } else {
                    is.push_back(new ISA::PushExceptionHandler2(
                        parseLocationReference(instructionLeader, tokens, startAt+i),
                        parseLocationReference(instructionLeader, tokens, startAt+i+1)
                    ));
                    i += 2;
                }
            } else if ( instructionLeader == "popexhandler" ) {
                is.push_back(new ISA::PopExceptionHandler(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "raise" ) {
                is.push_back(new ISA::Raise(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "resume" ) {
                is.push_back(new ISA::Resume(
                    parseLocationReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "otypeinit" ) {
                is.push_back(new ISA::OTypeInit());
            } else if ( instructionLeader == "otypeprop" ) {
                is.push_back(new ISA::OTypeProp(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseObjectPropertyReference(instructionLeader, tokens, startAt+i+1),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                ));
                i += 3;
            } else if ( instructionLeader == "otypedel" ) {
                is.push_back(new ISA::OTypeDel(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseObjectPropertyReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "otypeget" ) {
                is.push_back(new ISA::OTypeGet(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseObjectPropertyReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "otypefinalize" ) {
                is.push_back(new ISA::OTypeFinalize(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "otypesubset" ) {
                is.push_back(new ISA::OTypeSubset(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "objinit" ) {
                is.push_back(new ISA::ObjInit(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "objset" ) {
                is.push_back(new ISA::ObjSet(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseObjectPropertyReference(instructionLeader, tokens, startAt+i+1),
                    parseUnaryReference(instructionLeader, tokens, startAt+i+2)
                ));
                i += 3;
            } else if ( instructionLeader == "objget" ) {
                is.push_back(new ISA::ObjGet(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseObjectPropertyReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else if ( instructionLeader == "objinstance" ) {
                is.push_back(new ISA::ObjInstance(
                    parseUnaryReference(instructionLeader, tokens, startAt+i)
                ));
                i += 1;
            } else if ( instructionLeader == "objcurry" ) {
                is.push_back(new ISA::ObjCurry(
                    parseUnaryReference(instructionLeader, tokens, startAt+i),
                    parseObjectPropertyReference(instructionLeader, tokens, startAt+i+1)
                ));
                i += 2;
            } else {
                throw Errors::SwarmError("Malformed instruction: `" + instructionLeader + "` (unknown instruction tag)");
            }

            return i;
        }

        /**
         * Parse a single assignment instruction from the list of tokens, starting at the `startAt`-th token.
         * Returns the number of tokens which were consumed from `tokens`.
         */
        virtual std::size_t parseAssignment(Instructions& is, std::vector<std::string>& tokens, std::size_t startAt) {
            // An assignment must consist of <lval> <larrow> <reference|instruction>

            // Parse the lval
            std::size_t i = 1;
            auto lvalToken = tokens.at(startAt);
            auto lval = parseLocation(lvalToken);

            // Parse the larrow
            if ( tokens.size() <= startAt+i ) throw Errors::SwarmError("Malformed assignment to `" + lvalToken + "` (expected <-, got EOF)");
            auto arrowToken = tokens.at(startAt+i);
            if ( arrowToken != "<-" ) throw Errors::SwarmError("Malformed assignment to `" + lvalToken + "` (expected <-, got " + arrowToken + ")");
            i += 1;

            // Parse the reference or instruction
            if ( tokens.size() <= startAt+i ) throw Errors::SwarmError("Malformed assignment to `" + lvalToken + "` (expected rval, got EOF)");
            auto rvalLeader = tokens.at(startAt+i);

            if ( isReferenceLeader(rvalLeader) ) {
                // This is a reference, which is always a single token
                auto ref = parseReference(rvalLeader);
                i += 1;
                is.push_back(new AssignValue(lval, ref));
            } else {
                // This is an instruction, which may consist of multiple tokens
                Instructions tmpIs;
                i += parseInstruction(tmpIs, tokens, startAt+i);
                is.push_back(new AssignEval(lval, tmpIs.at(0)));
            }

            return i;
        }

        /** Parse the unary reference represented by the single token. */
        virtual ISA::Reference* parseReference(std::string const& token) {
            if ( isLocationLeader(token) ) {
                return parseLocation(token);
            }

            if ( isTypeLeader(token) ) {
                return parseType(token);
            }

            if ( token.at(0) == '"' ) {
                return new ISA::StringReference(token.substr(1, token.length() - 2));
            }

            if ( std::isdigit(token.at(0)) ) {
                try {
                    auto value = std::stod(token);
                    return new ISA::NumberReference(value);
                } catch (std::invalid_argument& e) {
                    throw Errors::SwarmError("Malformed number reference `" + token + "`");
                }
            }

            if ( token == "true" || token == "false" ) {
                return new ISA::BooleanReference(token == "true");
            }

            throw Errors::SwarmError("Malformed reference `" + token + "`");
        }

        /**
         * Parse a location reference from the list of tokens starting at the `startAt`-th token.
         * Returns the ISA::LocationReference instance, and consumes a single token.
         */
        virtual ISA::LocationReference* parseLocationReference(std::string const& leader, std::vector<std::string>& tokens, std::size_t startAt) {
            if ( tokens.size() < startAt+1 ) throw Errors::SwarmError("Malformed instruction `" + leader + "` (expected 1 reference, got EOF)");
            auto token = tokens.at(startAt);
            auto r = parseReference(token);
            if ( r->tag() != ReferenceTag::LOCATION ) throw Errors::SwarmError("Malformed instruction: `" + leader + "` (expected location, got `" + token + "`");
            return (ISA::LocationReference*) r;
        }

        /**
         * Parse a location reference from the list of tokens starting at the `startAt`-th token,
         * validating that the reference is an object-property affinity.
         */
        virtual ISA::LocationReference* parseObjectPropertyReference(std::string const& leader, std::vector<std::string>& tokens, std::size_t startAt) {
            auto ref = parseLocationReference(leader, tokens, startAt);
            if ( ref->affinity() != Affinity::OBJECTPROP ) throw Errors::SwarmError("Malformed instruction: `" + leader + "` (expected object property name, got affinity `" + LocationReference::affinityString(ref->affinity()) + "`");
            return ref;
        }

        /** Returns true if the leading string of a token represents a LocationReference. */
        virtual bool isLocationLeader(std::string const& leader) {
            if ( leader.at(0) == '$' ) return true;
            if ( leader == "p:MAP" ) return true;
            if ( leader == "p:ENUM" ) return true;
            if ( leader == "p:LAMBDA0" ) return true;
            if ( leader == "p:LAMBDA" ) return true;
            return (
                leader.size() > 1
                && (leader.at(0) == 'f' || leader.at(0) == 'o')
                && leader.at(1) == ':'
            );
        }

        /** Returns true if the leading string of a token represents a type reference. */
        virtual bool isTypeLeader(std::string const& leader) {
            return (
                leader.size() > 1
                && leader.at(0) == 'p'
                && leader.at(1) == ':'
                && leader != "p:MAP"
                && leader != "p:ENUM"
                && leader != "p:LAMBDA0"
                && leader != "p:LAMBDA"
            );
        }

        /** Returns true if the leading string of a token represents a reference literal of some kind. */
        virtual bool isReferenceLeader(std::string const& leader) {
            // A leader for a reference must be any of:
            // - the beginning of a string
            if ( leader.at(0) == '"' ) return true;

            // - numeric
            if ( std::isdigit(leader.at(0)) ) return true;

            // - the start of an lval
            if ( isLocationLeader(leader) ) return true;

            // - the start of a type
            if ( isTypeLeader(leader) ) return true;

            // - true
            // - false
            return leader == "true" || leader == "false";
        }

        /** Parse the TypeReference represented by the given token. */
        virtual ISA::TypeReference* parseType(std::string const& token) {
            Type::Type* type;

            if ( token == "p:TYPE" ) type = Type::Primitive::of(Type::Intrinsic::TYPE);
            else if ( token == "p:VOID" ) type = Type::Primitive::of(Type::Intrinsic::VOID);
            else if ( token == "p:NUMBER" ) type = Type::Primitive::of(Type::Intrinsic::NUMBER);
            else if ( token == "p:STRING" ) type = Type::Primitive::of(Type::Intrinsic::STRING);
            else if ( token == "p:BOOLEAN" ) type = Type::Primitive::of(Type::Intrinsic::BOOLEAN);
            else if ( token == "p:THIS" ) type = Type::Primitive::of(Type::Intrinsic::THIS);
            else throw Errors::SwarmError("Malformed primitive type name: `" + token + "` (expected one of p:TYPE, p:NUMBER, p:STRING, p:BOOLEAN, p:VOID, p:THIS)");

            return new ISA::TypeReference(type);
        }

        /** Parse the LocationReference represented by the given token. */
        static ISA::LocationReference* parseLocation(std::string const& token) {
            if ( token.at(0) == '$' && (token.length() < 4 || token.at(2) != ':') ) throw Errors::SwarmError("Malformed location reference: `" + token + "` (expected form $_:_)");
            if ( token.at(0) != '$' && (token.length() < 3 || token.at(1) != ':') ) throw Errors::SwarmError("Malformed location reference: `" + token + "` (expected form f:_ or o:_ or p:_)");

            auto location = token.at(0) == '$' ? token.at(1) : token.at(0);
            auto name = token.substr(token.at(0) == '$' ? 3 : 2);

            Affinity a;
            if ( location == 'l' ) a = Affinity::LOCAL;
            else if ( location == 's' ) a = Affinity::SHARED;
            else if ( location == 'f' ) a = Affinity::FUNCTION;
            else if ( location == 'p' ) a = Affinity::PRIMITIVE;
            else if ( location == 'o' ) a = Affinity::OBJECTPROP;
            else throw Errors::SwarmError("Invalid location affinity: `" + token + "` (expected l or s or f or p or o)");

            return new ISA::LocationReference(a, name);
        }

        /** Print the parsed instructions to the given stream. */
        virtual void outputParse(std::ostream& out) {
            auto is = parse();
            for ( const auto& i : is ) {
                out << i->toString() << std::endl;
            }
            dispose(is);
        }

        /** Print the loaded tokens to the given stream. */
        virtual void outputTokens(std::ostream& out) {
            auto tokens = tokenize();
            for ( const auto& token : tokens ) {
                out << token << std::endl;
            }
        }

        /** Free the memory used by the Instruction instances in `is`. */
        virtual void dispose(Instructions& is) {
            for ( auto i : is ) {
                delete i;
            }
        }

        /**
         * Returns the number of operands present in the token stream before another instruction
         * is encountered using a look-ahead, starting at the `startAt`-th token.
         */
        virtual std::size_t countOperands(std::vector<std::string> const& tokens, std::size_t startAt) {
            std::size_t n = 0;

            for ( std::size_t i = startAt; i < tokens.size(); i += 1 ) {
                if ( isReferenceLeader(tokens.at(i)) ) {
                    n += 1;
                    continue;
                }

                break;
            }

            /* Special case:
             * call f:MY_FUN
             * $l:a <- 3
             * Currently, calling countOperands after `call` will return 2, since $l:a
             * is a valid operand. So, resolve this by checking if the next token is <-. */

            if ( tokens.size() > (startAt+n) ) {
                if ( tokens.at(startAt+n) == "<-" ) {
                    // The last token we found is the lval of an assignment
                    n -= 1;
                }
            }

            return n;
        }

    protected:
        std::istream& _in;
    };

}

#endif //SWARMVM_ISAPARSER
