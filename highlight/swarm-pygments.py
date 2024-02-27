from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

import re

__all__=['SwarmLexer']

class SwarmLexer(RegexLexer):
    name = 'Swarm'
    aliases = ['swarm']
    filenames = ['*.swarm']
    flags = re.MULTILINE | re.UNICODE

    tokens = {
        'root' : [
            (u'(\\-\\-\\*)', bygroups(Comment), 'main__1'),
            (u'(\\-\\-[^\\*\\n\\r].*)', bygroups(Comment)),
            (u'(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)', bygroups(Name.Function)),
            (u'(enumerate|with|while|if|as|include|from|shared|constructor)', bygroups(Keyword)),
            (u'(number|bool|map|string|enumerable|fn|type)', bygroups(Keyword.Type)),
            (u'(\\b[a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Name.Variable)),
            (u'(\\b\\d+\\.\\d+)', bygroups(Number)),
            (u'(\\b\\d+)', bygroups(Number)),
            (u'(\\{)', bygroups(Punctuation), 'main__2'),
            (u'(;|\\.|\\^|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!|:|&|\\||%)', bygroups(Punctuation)),
            (u'(\\\")', bygroups(String), 'main__3'),
            (u'(\\()', bygroups(Punctuation), 'main__4'),
            (u'([^\\s\\n\\r])', bygroups(Generic.Error)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__1' : [
            ('(\n|\r|\r\n)', String),
            ('.', Comment),
        ],
        'main__2' : [
            (u'(\\-\\-\\*)', bygroups(Comment), 'main__1'),
            (u'(\\-\\-[^\\*\\n\\r].*)', bygroups(Comment)),
            (u'(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)', bygroups(Name.Function)),
            (u'(enumerate|with|while|if|as|include|from|shared|constructor)', bygroups(Keyword)),
            (u'(number|bool|map|string|enumerable|fn|type)', bygroups(Keyword.Type)),
            (u'(\\b[a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Name.Variable)),
            (u'(\\b\\d+\\.\\d+)', bygroups(Number)),
            (u'(\\b\\d+)', bygroups(Number)),
            (u'(\\{)', bygroups(Punctuation), 'main__2'),
            (u'(;|\\.|\\^|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!|:|&|\\||%)', bygroups(Punctuation)),
            (u'(\\\")', bygroups(String), 'main__3'),
            (u'(\\()', bygroups(Punctuation), 'main__4'),
            (u'([^\\s\\n\\r])', bygroups(Generic.Error)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__3' : [
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__4' : [
            (u'(\\-\\-\\*)', bygroups(Comment), 'main__1'),
            (u'(\\-\\-[^\\*\\n\\r].*)', bygroups(Comment)),
            (u'(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)', bygroups(Name.Function)),
            (u'(enumerate|with|while|if|as|include|from|shared|constructor)', bygroups(Keyword)),
            (u'(number|bool|map|string|enumerable|fn|type)', bygroups(Keyword.Type)),
            (u'(\\b[a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Name.Variable)),
            (u'(\\b\\d+\\.\\d+)', bygroups(Number)),
            (u'(\\b\\d+)', bygroups(Number)),
            (u'(\\{)', bygroups(Punctuation), 'main__2'),
            (u'(;|\\.|\\^|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!|:|&|\\||%)', bygroups(Punctuation)),
            (u'(\\\")', bygroups(String), 'main__3'),
            (u'(\\()', bygroups(Punctuation), 'main__4'),
            (u'([^\\s\\n\\r])', bygroups(Generic.Error)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ]
    }

