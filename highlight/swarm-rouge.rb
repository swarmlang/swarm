# -*- coding: utf-8 -*- #

module Rouge
  module Lexers
    class Swarm < RegexLexer
      title     "swarm"
      tag       'Swarm'
      mimetypes 'text/x-swarm'
      filenames '*.swarm'

      state:root do
          rule /(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)/, Name::Function
          rule /(enumerate|with|while|if|as)/, Keyword
          rule /(number|bool|map|string|enumerable)/, Keyword::Type
          rule /(\b[a-zA-Z_][a-zA-Z0-9_]*)/, Name::Variable
          rule /(\b\d+\.\d+)/, Number
          rule /(\b\d+)/, Number
          rule /(\{)/, Punctuation, :main__1
          rule /(;|\.|\^|,|<|>|=|\[|\]|\+|-|\*|\\/|\!)/, Punctuation
          rule /(\")/, String, :main__2
          rule /(\()/, Punctuation, :main__3
          rule /(\/\/.*)/, Comment
          rule /([^\s\n\r]/, Generic::Error
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

      state:main__1 do
          rule /(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)/, Name::Function
          rule /(enumerate|with|while|if|as)/, Keyword
          rule /(number|bool|map|string|enumerable)/, Keyword::Type
          rule /(\b[a-zA-Z_][a-zA-Z0-9_]*)/, Name::Variable
          rule /(\b\d+\.\d+)/, Number
          rule /(\b\d+)/, Number
          rule /(\{)/, Punctuation, :main__1
          rule /(;|\.|\^|,|<|>|=|\[|\]|\+|-|\*|\\/|\!)/, Punctuation
          rule /(\")/, String, :main__2
          rule /(\()/, Punctuation, :main__3
          rule /(\/\/.*)/, Comment
          rule /([^\s\n\r]/, Generic::Error
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

      state:main__2 do
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

      state:main__3 do
          rule /(random|randomVector|randomMatrix|log|logError|numberToString|boolToString|min|max|fileContents|range|tag|shell|sin)/, Name::Function
          rule /(enumerate|with|while|if|as)/, Keyword
          rule /(number|bool|map|string|enumerable)/, Keyword::Type
          rule /(\b[a-zA-Z_][a-zA-Z0-9_]*)/, Name::Variable
          rule /(\b\d+\.\d+)/, Number
          rule /(\b\d+)/, Number
          rule /(\{)/, Punctuation, :main__1
          rule /(;|\.|\^|,|<|>|=|\[|\]|\+|-|\*|\\/|\!)/, Punctuation
          rule /(\")/, String, :main__2
          rule /(\()/, Punctuation, :main__3
          rule /(\/\/.*)/, Comment
          rule /([^\s\n\r]/, Generic::Error
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

    end
  end
end

