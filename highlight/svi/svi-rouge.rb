# -*- coding: utf-8 -*- #

module Rouge
  module Lexers
    class Svi < RegexLexer
      title     "svi"
      tag       'Svi'
      mimetypes 'text/x-svi'
      filenames '*.svi'

      state:root do
          rule /((\$l|\$s):[a-zA-Z0-9_]*)/, Name::Variable
          rule /((o|p):[a-zA-Z0-9_]*)/, Keyword::Type
          rule /(f:[a-zA-Z0-9_]*)/, Name::Function
          rule /(<\-)/, Punctuation
          rule /(true|false)/, Keyword
          rule /(beginfn|fnparam|return|curry|callif|callelse|call|pushcallif|pushcallelse|pushcall|drain|exit|out|err|streaminit|streampush|streampop|streamclose|streamempty|typify|lock|unlock|equal|scopeof|typeof|compatible|nand|and|xor|nor|or|not|mapinit|mapset|mapget|maplength|mapkeys|enuminit|enumappend|enumprepend|enumlength|enumget|enumset|enumerate|strconcat|strlength|strslice|plus|minus|times|divide|power|mod|neg|gte|gt|lte|lt|while|with|pushexhandler|popexhandler|raise|resume|otypeinit|otypeprop|otypedel|otypeget|otypefinalize|otypesubset|objinit|objset|objget|objinstance|objcurry)/, Keyword
          rule /(\b\d+\.\d+)/, Number
          rule /(\b\d+)/, Number
          rule /(\")/, String, :main__1
          rule /(\-\-.*)/, Comment
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

      state:main__1 do
          rule /(\n|\r|\r\n)/, String
          rule /./, String
      end

    end
  end
end

