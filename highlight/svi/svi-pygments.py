from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

import re

__all__=['SviLexer']

class SviLexer(RegexLexer):
    name = 'Svi'
    aliases = ['svi']
    filenames = ['*.svi']
    flags = re.MULTILINE | re.UNICODE

    tokens = {
        'root' : [
            (u'((\\$l|\\$s):[a-zA-Z0-9_]*)', bygroups(Name.Variable)),
            (u'((o|p):[a-zA-Z0-9_]*)', bygroups(Keyword.Type)),
            (u'(f:[a-zA-Z0-9_]*)', bygroups(Name.Function)),
            (u'(<\\-)', bygroups(Punctuation)),
            (u'(true|false)', bygroups(Keyword)),
            (u'(beginfn|fnparam|return|curry|callif|callelse|call|pushcallif|pushcallelse|pushcall|drain|exit|out|err|streaminit|streampush|streampop|streamclose|streamempty|typify|lock|unlock|equal|scopeof|typeof|compatible|nand|and|xor|nor|or|not|mapinit|mapset|mapget|maplength|mapkeys|enuminit|enumappend|enumprepend|enumlength|enumget|enumset|enumerate|strconcat|strlength|strslice|plus|minus|times|divide|power|mod|neg|gte|gt|lte|lt|while|with|pushexhandler|popexhandler|raise|resume|otypeinit|otypeprop|otypedel|otypeget|otypefinalize|otypesubset|objinit|objset|objget|objinstance|objcurry)', bygroups(Keyword)),
            (u'(\\b\\d+\\.\\d+)', bygroups(Number)),
            (u'(\\b\\d+)', bygroups(Number)),
            (u'(\\\")', bygroups(String), 'main__1'),
            (u'(\\-\\-.*)', bygroups(Comment)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__1' : [
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ]
    }

