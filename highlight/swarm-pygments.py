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
            (u'(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)', bygroups(Keyword)),
            (u'(enumerate|with|while|if|as)', bygroups(Keyword)),
            (u'(number|bool|map|string|enumerable)', bygroups(Keyword)),
            (u'(\\b[a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Keyword)),
            (u'(\\b\\d+\\.\\d+)', bygroups(Number)),
            (u'(\\b\\d+)', bygroups(Number)),
            (u'(\\{)', bygroups(Punctuation), 'main__1'),
            (u'(;|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!)', bygroups(Punctuation)),
            (u'(\\\")', bygroups(Punctuation), 'main__2'),
            (u'(\\()', bygroups(Punctuation), 'main__3'),
            (u'(//.*)', bygroups(Comment)),
            (u'([^\\s\\n\\r])', bygroups(Generic.Error)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__1' : [
            (u'(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)', bygroups(Keyword)),
            (u'(enumerate|with|while|if|as)', bygroups(Keyword)),
            (u'(number|bool|map|string|enumerable)', bygroups(Keyword)),
            (u'(\\b[a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Keyword)),
            (u'(\\b\\d+\\.\\d+)', bygroups(Number)),
            (u'(\\b\\d+)', bygroups(Number)),
            (u'(\\{)', bygroups(Punctuation), 'main__1'),
            (u'(;|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!)', bygroups(Punctuation)),
            (u'(\\\")', bygroups(Punctuation), 'main__2'),
            (u'(\\()', bygroups(Punctuation), 'main__3'),
            (u'(//.*)', bygroups(Comment)),
            (u'([^\\s\\n\\r])', bygroups(Generic.Error)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__2' : [
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ],
        'main__3' : [
            (u'(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)', bygroups(Keyword)),
            (u'(enumerate|with|while|if|as)', bygroups(Keyword)),
            (u'(number|bool|map|string|enumerable)', bygroups(Keyword)),
            (u'(\\b[a-zA-Z_][a-zA-Z0-9_]*)', bygroups(Keyword)),
            (u'(\\b\\d+\\.\\d+)', bygroups(Number)),
            (u'(\\b\\d+)', bygroups(Number)),
            (u'(\\{)', bygroups(Punctuation), 'main__1'),
            (u'(;|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!)', bygroups(Punctuation)),
            (u'(\\\")', bygroups(Punctuation), 'main__2'),
            (u'(\\()', bygroups(Punctuation), 'main__3'),
            (u'(//.*)', bygroups(Comment)),
            (u'([^\\s\\n\\r])', bygroups(Generic.Error)),
            ('(\n|\r|\r\n)', String),
            ('.', String),
        ]
    }


