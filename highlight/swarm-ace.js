/*
* To try in Ace editor, copy and paste into the mode creator
* here : http://ace.c9.io/tool/mode_creator.html
*/

define(function(require, exports, module) {
   "use strict";
   var oop = require("../lib/oop");
   var TextHighlightRules = require("./text_highlight_rules").TextHighlightRules;
   /* --------------------- START ----------------------------- */
   var SwarmHighlightRules = function() {
      this.$rules = {
         "start" : [
            {
               "token" : "comment",
               "regex" : "(\\-\\-\\*)",
               "push" : "main__1"
            },
            {
               "token" : "comment",
               "regex" : "(\\-\\-[^\\*].*)"
            },
            {
               "token" : "entity.name.function",
               "regex" : "(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)"
            },
            {
               "token" : "keyword",
               "regex" : "(enumerate|with|while|if|as|include|from|shared|constructor)"
            },
            {
               "token" : "entity.name.type",
               "regex" : "(number|bool|map|string|enumerable|fn|type)"
            },
            {
               "token" : "variable",
               "regex" : "(\\b[a-zA-Z_][a-zA-Z0-9_]*)"
            },
            {
               "token" : "constant.numeric",
               "regex" : "(\\b\\d+\\.\\d+)"
            },
            {
               "token" : "constant.numeric",
               "regex" : "(\\b\\d+)"
            },
            {
               "token" : "punctuation",
               "regex" : "(\\{)",
               "push" : "main__2"
            },
            {
               "token" : "punctuation",
               "regex" : "(;|\\.|\\^|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!|:|&|\\||%)"
            },
            {
               "token" : "string",
               "regex" : "(\\\")",
               "push" : "main__3"
            },
            {
               "token" : "punctuation",
               "regex" : "(\\()",
               "push" : "main__4"
            },
            {
               "token" : "invalid",
               "regex" : "([^\\s])"
            },
            {
               defaultToken : "text",
            }
         ],
         "main__1" : [
            {
               "token" : "comment",
               "regex" : "(\\*\\-\\-)",
               "next" : "pop"
            },
            {
               defaultToken : "comment",
            }
         ],
         "main__2" : [
            {
               "token" : "punctuation",
               "regex" : "(\\})",
               "next" : "pop"
            },
            {
               "token" : "comment",
               "regex" : "(\\-\\-\\*)",
               "push" : "main__1"
            },
            {
               "token" : "comment",
               "regex" : "(\\-\\-[^\\*].*)"
            },
            {
               "token" : "entity.name.function",
               "regex" : "(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)"
            },
            {
               "token" : "keyword",
               "regex" : "(enumerate|with|while|if|as|include|from|shared|constructor)"
            },
            {
               "token" : "entity.name.type",
               "regex" : "(number|bool|map|string|enumerable|fn|type)"
            },
            {
               "token" : "variable",
               "regex" : "(\\b[a-zA-Z_][a-zA-Z0-9_]*)"
            },
            {
               "token" : "constant.numeric",
               "regex" : "(\\b\\d+\\.\\d+)"
            },
            {
               "token" : "constant.numeric",
               "regex" : "(\\b\\d+)"
            },
            {
               "token" : "punctuation",
               "regex" : "(\\{)",
               "push" : "main__2"
            },
            {
               "token" : "punctuation",
               "regex" : "(;|\\.|\\^|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!|:|&|\\||%)"
            },
            {
               "token" : "string",
               "regex" : "(\\\")",
               "push" : "main__3"
            },
            {
               "token" : "punctuation",
               "regex" : "(\\()",
               "push" : "main__4"
            },
            {
               "token" : "invalid",
               "regex" : "([^\\s])"
            },
            {
               defaultToken : "text",
            }
         ],
         "main__3" : [
            {
               "token" : "string",
               "regex" : "(\\\")",
               "next" : "pop"
            },
            {
               defaultToken : "string",
            }
         ],
         "main__4" : [
            {
               "token" : "punctuation",
               "regex" : "(\\))",
               "next" : "pop"
            },
            {
               "token" : "comment",
               "regex" : "(\\-\\-\\*)",
               "push" : "main__1"
            },
            {
               "token" : "comment",
               "regex" : "(\\-\\-[^\\*].*)"
            },
            {
               "token" : "entity.name.function",
               "regex" : "(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)"
            },
            {
               "token" : "keyword",
               "regex" : "(enumerate|with|while|if|as|include|from|shared|constructor)"
            },
            {
               "token" : "entity.name.type",
               "regex" : "(number|bool|map|string|enumerable|fn|type)"
            },
            {
               "token" : "variable",
               "regex" : "(\\b[a-zA-Z_][a-zA-Z0-9_]*)"
            },
            {
               "token" : "constant.numeric",
               "regex" : "(\\b\\d+\\.\\d+)"
            },
            {
               "token" : "constant.numeric",
               "regex" : "(\\b\\d+)"
            },
            {
               "token" : "punctuation",
               "regex" : "(\\{)",
               "push" : "main__2"
            },
            {
               "token" : "punctuation",
               "regex" : "(;|\\.|\\^|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!|:|&|\\||%)"
            },
            {
               "token" : "string",
               "regex" : "(\\\")",
               "push" : "main__3"
            },
            {
               "token" : "punctuation",
               "regex" : "(\\()",
               "push" : "main__4"
            },
            {
               "token" : "invalid",
               "regex" : "([^\\s])"
            },
            {
               defaultToken : "text",
            }
         ]
      };
      this.normalizeRules();
   };
   /* ------------------------ END ------------------------------ */
   oop.inherits(SwarmHighlightRules, TextHighlightRules);
   exports.SwarmHighlightRules = SwarmHighlightRules;
});
