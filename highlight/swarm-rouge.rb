# -*- coding: utf-8 -*- #

module Rouge
  module Lexers
    class Swarm < RegexLexer
      title     "swarm"
      tag       'Swarm'
      mimetypes 'text/x-swarm'
      filenames '*.swarm'

      state:root do
          rule /(\-\-\*)/, Comment, :main__1
          rule /(\-\-[^\*\n\r].*)/, Comment
          rule /(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)/, Name::Function
          rule /(enumerate|with|while|if|as|include|from|shared|constructor)/, Keyword
          rule /(number|bool|map|string|enumerable|fn|type)/, Keyword::Type
          rule /(\b[a-zA-Z_][a-zA-Z0-9_]*)/, Name::Variable
          rule /(\b\d+\.\d+)/, Number
          rule /(\b\d+)/, Number
          rule /(\{)/, Punctuation, :main__2
          rule /(;|\.|\^|,|<|>|=|\[|\]|\+|-|\*|\\/|\!|:|&|\||%)/, Punctuation
          rule /(\")/, String, :main__3
          rule /(\()/, Punctuation, :main__4
          rule /([^\s\n\r]/, Generic::Error
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

      state:main__1 do
          rule /(\n|\r|\r\n)/, String
          rule /./, Comment
      end

      state:main__2 do
          rule /(\-\-\*)/, Comment, :main__1
          rule /(\-\-[^\*\n\r].*)/, Comment
          rule /(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)/, Name::Function
          rule /(enumerate|with|while|if|as|include|from|shared|constructor)/, Keyword
          rule /(number|bool|map|string|enumerable|fn|type)/, Keyword::Type
          rule /(\b[a-zA-Z_][a-zA-Z0-9_]*)/, Name::Variable
          rule /(\b\d+\.\d+)/, Number
          rule /(\b\d+)/, Number
          rule /(\{)/, Punctuation, :main__2
          rule /(;|\.|\^|,|<|>|=|\[|\]|\+|-|\*|\\/|\!|:|&|\||%)/, Punctuation
          rule /(\")/, String, :main__3
          rule /(\()/, Punctuation, :main__4
          rule /([^\s\n\r]/, Generic::Error
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

      state:main__3 do
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

      state:main__4 do
          rule /(\-\-\*)/, Comment, :main__1
          rule /(\-\-[^\*\n\r].*)/, Comment
          rule /(numberToString|booleanToString|vectorToString|matrixToString|sin|cos|tan|random|randomVector|randomMatrix|zeroVector|zeroMatrix|range|lLog|sLog|lError|sError|floor|ceiling|max|min|nthRoot|count|time|subVector|subMatrix|tag|open|read|write|append)/, Name::Function
          rule /(enumerate|with|while|if|as|include|from|shared|constructor)/, Keyword
          rule /(number|bool|map|string|enumerable|fn|type)/, Keyword::Type
          rule /(\b[a-zA-Z_][a-zA-Z0-9_]*)/, Name::Variable
          rule /(\b\d+\.\d+)/, Number
          rule /(\b\d+)/, Number
          rule /(\{)/, Punctuation, :main__2
          rule /(;|\.|\^|,|<|>|=|\[|\]|\+|-|\*|\\/|\!|:|&|\||%)/, Punctuation
          rule /(\")/, String, :main__3
          rule /(\()/, Punctuation, :main__4
          rule /([^\s\n\r]/, Generic::Error
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

    end
  end
end

