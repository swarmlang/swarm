#include "ISA.h"

namespace swarmc::ISA {

    std::string Instruction::tagName(Tag tag) {
        if ( tag == Tag::BEGINFN ) return "BEGINFN";
        if ( tag == Tag::FNPARAM ) return "FNPARAM";
        if ( tag == Tag::RETURN0 ) return "RETURN0";
        if ( tag == Tag::RETURN1 ) return "RETURN1";
        if ( tag == Tag::CURRY ) return "CURRY";
        if ( tag == Tag::CALL0 ) return "CALL0";
        if ( tag == Tag::CALL1 ) return "CALL1";
        if ( tag == Tag::CALLIF0 ) return "CALLIF0";
        if ( tag == Tag::CALLIF1 ) return "CALLIF1";
        if ( tag == Tag::CALLELSE0 ) return "CALLELSE0";
        if ( tag == Tag::CALLELSE1 ) return "CALLELSE1";
        if ( tag == Tag::PUSHCALL0 ) return "PUSHCALL0";
        if ( tag == Tag::PUSHCALL1 ) return "PUSHCALL1";
        if ( tag == Tag::PUSHCALLIF0 ) return "PUSHCALLIF0";
        if ( tag == Tag::PUSHCALLIF1 ) return "PUSHCALLIF1";
        if ( tag == Tag::PUSHCALLELSE0 ) return "PUSHCALLELSE0";
        if ( tag == Tag::PUSHCALLELSE1 ) return "PUSHCALLELSE1";
        if ( tag == Tag::OUT ) return "OUT";
        if ( tag == Tag::ERR ) return "ERR";
        if ( tag == Tag::STREAMINIT ) return "STREAMINIT";
        if ( tag == Tag::STREAMPUSH ) return "STREAMPUSH";
        if ( tag == Tag::STREAMPOP ) return "STREAMPOP";
        if ( tag == Tag::STREAMCLOSE ) return "STREAMCLOSE";
        if ( tag == Tag::STREAMEMPTY ) return "STREAMEMPTY";
        if ( tag == Tag::TYPIFY ) return "TYPIFY";
        if ( tag == Tag::ASSIGNVALUE ) return "ASSIGNVALUE";
        if ( tag == Tag::ASSIGNEVAL ) return "ASSIGNEVAL";
        if ( tag == Tag::LOCK ) return "LOCK";
        if ( tag == Tag::UNLOCK ) return "UNLOCK";
        if ( tag == Tag::EQUAL ) return "EQUAL";
        if ( tag == Tag::SCOPEOF ) return "SCOPEOF";
        if ( tag == Tag::TYPEOF ) return "TYPEOF";
        if ( tag == Tag::COMPATIBLE ) return "COMPATIBLE";
        if ( tag == Tag::AND ) return "AND";
        if ( tag == Tag::OR ) return "OR";
        if ( tag == Tag::XOR ) return "XOR";
        if ( tag == Tag::NAND ) return "NAND";
        if ( tag == Tag::NOR ) return "NOR";
        if ( tag == Tag::NOT ) return "NOT";
        if ( tag == Tag::MAPINIT ) return "MAPINIT";
        if ( tag == Tag::MAPSET ) return "MAPSET";
        if ( tag == Tag::MAPGET ) return "MAPGET";
        if ( tag == Tag::MAPLENGTH ) return "MAPLENGTH";
        if ( tag == Tag::MAPKEYS ) return "MAPKEYS";
        if ( tag == Tag::ENUMINIT ) return "ENUMINIT";
        if ( tag == Tag::ENUMAPPEND ) return "ENUMAPPEND";
        if ( tag == Tag::ENUMPREPEND ) return "ENUMPREPEND";
        if ( tag == Tag::ENUMLENGTH ) return "ENUMLENGTH";
        if ( tag == Tag::ENUMGET ) return "ENUMGET";
        if ( tag == Tag::ENUMSET ) return "ENUMSET";
        if ( tag == Tag::ENUMERATE ) return "ENUMERATE";
        if ( tag == Tag::STRCONCAT ) return "STRCONCAT";
        if ( tag == Tag::STRLENGTH ) return "STRLENGTH";
        if ( tag == Tag::STRSLICEFROM ) return "STRSLICEFROM";
        if ( tag == Tag::STRSLICEFROMTO ) return "STRSLICEFROMTO";
        if ( tag == Tag::PLUS ) return "PLUS";
        if ( tag == Tag::MINUS ) return "MINUS";
        if ( tag == Tag::TIMES ) return "TIMES";
        if ( tag == Tag::DIVIDE ) return "DIVIDE";
        if ( tag == Tag::POWER ) return "POWER";
        if ( tag == Tag::MOD ) return "MOD";
        if ( tag == Tag::NEG ) return "NEG";
        if ( tag == Tag::GT ) return "GT";
        if ( tag == Tag::GTE ) return "GTE";
        if ( tag == Tag::LT ) return "LT";
        if ( tag == Tag::LTE ) return "LTE";
        if ( tag == Tag::WHILE ) return "WHILE";
        if ( tag == Tag::WITH ) return "WITH";
        if ( tag == Tag::PUSHEXHANDLER1 ) return "PUSHEXHANDLER1";
        if ( tag == Tag::PUSHEXHANDLER2 ) return "PUSHEXHANDLER2";
        if ( tag == Tag::POPEXHANDLER ) return "POPEXHANDLER";
        if ( tag == Tag::RAISE ) return "RAISE";
        if ( tag == Tag::RESUME ) return "RESUME";
        return "UNKNOWN";
    }

}
