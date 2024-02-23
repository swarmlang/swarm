#include "ToISAWalk.h"

namespace swarmc::Lang::Walk {
    const std::unordered_map<std::string, ISA::LocationReference*> ToISAWalk::_opaqueVals = {
        { "Opaque<PROLOGUE::TAG>", new ISA::LocationReference(ISA::Affinity::FUNCTION, "TAG_T") },
        { "Opaque<PROLOGUE::FILE>", new ISA::LocationReference(ISA::Affinity::FUNCTION, "FILE_T") }
    };

    const std::unordered_map<std::string, ISA::LocationReference*> ToISAWalk::FuncToLocation = {
        { "lLog", new ISA::LocationReference(ISA::Affinity::LOCAL, "STDOUT") },
        { "lError", new ISA::LocationReference(ISA::Affinity::LOCAL, "STDERR") },
        { "sLog", new ISA::LocationReference(ISA::Affinity::SHARED, "STDOUT") },
        { "sError", new ISA::LocationReference(ISA::Affinity::SHARED, "STDERR") },
    };

    std::unordered_map<std::string, ISA::Instructions*> ToISAWalk::FuncToFunc;
}