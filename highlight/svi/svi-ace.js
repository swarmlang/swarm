/*
* To try in Ace editor, copy and paste into the mode creator
* here : http://ace.c9.io/tool/mode_creator.html
*/

define(function(require, exports, module) {
    "use strict";
    var oop = require("../lib/oop");
    var TextHighlightRules = require("./text_highlight_rules").TextHighlightRules;
    /* --------------------- START ----------------------------- */
    var SviHighlightRules = function() {
        this.$rules = {
            "start" : [
                {
                    "token" : "variable",
                    "regex" : "((\\$l|\\$s):[a-zA-Z0-9_]*)"
                },
                {
                    "token" : "entity.name.type",
                    "regex" : "((o|p):[a-zA-Z0-9_]*)"
                },
                {
                    "token" : "entity.name.function",
                    "regex" : "(f:[a-zA-Z0-9_]*)"
                },
                {
                    "token" : "punctuation",
                    "regex" : "(<\\-)"
                },
                {
                    "token" : "keyword",
                    "regex" : "(true|false)"
                },
                {
                    "token" : "keyword",
                    "regex" : "(beginfn|fnparam|return|curry|callif|callelse|call|pushcallif|pushcallelse|pushcall|drain|exit|out|err|streaminit|streampush|streampop|streamclose|streamempty|typify|lock|unlock|equal|scopeof|typeof|compatible|nand|and|xor|nor|or|not|mapinit|mapset|mapget|maplength|mapkeys|enuminit|enumappend|enumprepend|enumlength|enumget|enumset|enumerate|strconcat|strlength|strslice|plus|minus|times|divide|power|mod|neg|gte|gt|lte|lt|while|with|pushexhandler|popexhandler|raise|resume|otypeinit|otypeprop|otypedel|otypeget|otypefinalize|otypesubset|objinit|objset|objget|objinstance|objcurry)"
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
                    "token" : "string",
                    "regex" : "(\\\")",
                    "push" : "main__1"
                },
                {
                    "token" : "comment",
                    "regex" : "(\\-\\-.*)"
                },
                {
                    defaultToken : "text",
                }
            ],
            "main__1" : [
                {
                    "token" : "string",
                    "regex" : "(\\\")",
                    "next" : "pop"
                },
                {
                    defaultToken : "string",
                }
            ]
        };
        this.normalizeRules();
    };
    /* ------------------------ END ------------------------------ */
    oop.inherits(SviHighlightRules, TextHighlightRules);
    exports.SviHighlightRules = SviHighlightRules;
});
