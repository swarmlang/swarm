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
      "token" : "keyword",
      "regex" : "(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)"
   },
   {
      "token" : "keyword",
      "regex" : "(enumerate|with|while|if|as)"
   },
   {
      "token" : "keyword",
      "regex" : "(number|bool|map|string|enumerable)"
   },
   {
      "token" : "keyword",
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
      "push" : "main__1"
   },
   {
      "token" : "punctuation",
      "regex" : "(;|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!)"
   },
   {
      "token" : "punctuation",
      "regex" : "(\\\")",
      "push" : "main__2"
   },
   {
      "token" : "punctuation",
      "regex" : "(\\()",
      "push" : "main__3"
   },
   {
      "token" : "comment",
      "regex" : "(//.*)"
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
      "token" : "punctuation",
      "regex" : "(\\})",
      "next" : "pop"
   },
   {
      "token" : "keyword",
      "regex" : "(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)"
   },
   {
      "token" : "keyword",
      "regex" : "(enumerate|with|while|if|as)"
   },
   {
      "token" : "keyword",
      "regex" : "(number|bool|map|string|enumerable)"
   },
   {
      "token" : "keyword",
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
      "push" : "main__1"
   },
   {
      "token" : "punctuation",
      "regex" : "(;|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!)"
   },
   {
      "token" : "punctuation",
      "regex" : "(\\\")",
      "push" : "main__2"
   },
   {
      "token" : "punctuation",
      "regex" : "(\\()",
      "push" : "main__3"
   },
   {
      "token" : "comment",
      "regex" : "(//.*)"
   },
   {
      "token" : "invalid",
      "regex" : "([^\\s])"
   },
   {
      defaultToken : "text",
   }
],
"main__2" : [
   {
      "token" : "punctuation",
      "regex" : "(\\\")",
      "next" : "pop"
   },
   {
      defaultToken : "text",
   }
],
"main__3" : [
   {
      "token" : "punctuation",
      "regex" : "(\\))",
      "next" : "pop"
   },
   {
      "token" : "keyword",
      "regex" : "(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)"
   },
   {
      "token" : "keyword",
      "regex" : "(enumerate|with|while|if|as)"
   },
   {
      "token" : "keyword",
      "regex" : "(number|bool|map|string|enumerable)"
   },
   {
      "token" : "keyword",
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
      "push" : "main__1"
   },
   {
      "token" : "punctuation",
      "regex" : "(;|,|<|>|=|\\[|\\]|\\+|-|\\*|\\/|\\!)"
   },
   {
      "token" : "punctuation",
      "regex" : "(\\\")",
      "push" : "main__2"
   },
   {
      "token" : "punctuation",
      "regex" : "(\\()",
      "push" : "main__3"
   },
   {
      "token" : "comment",
      "regex" : "(//.*)"
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

