#pragma once

const char* test_input = R"(-- this test contains every token
--*
multiline * comment *-
ends *-*
here
*--
include Other.File;
from Other.Other.File include { some, ids };
from Other.Other.File include id2;

fn log = (s: string): void => {};

enumerable<number> t1 =  [1,2,3];
shared enumerable<enumerable<string>> t2 = [] of enumerable<string>;
number t3 = 4;
shared string t4 = "zoinks";
map<number->number> t5 = {} of number->number;
shared map<enumerable<number->string>> t6 = {
    a: [(arg1: number): string => "hello"],
    b: [(arg1: number): string => "goodbye", (arg1:number):string=>"zoinks"]
};

enumerate t6{b} as joe {
    with tag("mama", "is a nice woman") as _ {
        t1[1] = 4;
    }
}

enumerate t6{b} as shared var {
    with tag("tag", "str") as shared w {
        log("string");
    }
}

enumerate t6{b} as v, i {
}

enumerate t6{b} as shared v, i {
}

enumerate t6{b} as v, shared i {
}

enumerate t6{b} as shared v, shared i {
}

while (t3 > 0 && t3 >= -1 && t4 != "jinkies" && t3 < 5 && t3 <= 4) {
    if (t3 == 2) {
        continue;
    }
    if (t3 == 1) {
        fn func = (): void => {
            return;
        };
        shared fn func2 = (): bool => {
            return !(true || false);
        };

        break;
    }
    t3 += 0.1;
    t3 -= 1.1;
    t3 *= 1;
    t3 /= 1.0;
    t3 ^= 1;
    t3 %= 1;
    t4 += "s";
}

t3 = 1 + (2 - 3) / -4 % 1 ^ 5;
bool t7 = true;
t7 &&= false;
t7 ||= true;
t4 = t4 + t4;

fn t8 = (arg1: number, arg2: type): type => {
    return number;
};

shared fn t9 = (): type => number;

t8 = ((): number->type->type => (a: number, b: type): type => string)();
type->type t11 = t8(69);

type UserDefined = {
    constructor() => {
        uninit = 5;
    };

    number uninit;

    constructor(n: number) => {
        uninit = 5;
        if (n > 5) {
            uninit = n;
        }
    };

    fn regFunc = (): number->UserDefined => {
        return (i: number): UserDefined => UserDefined(i);
    };

    fn copy = (orig: UserDefined): UserDefined => {
        return UserDefined(orig.uninit);
    };
};

UserDefined obj = UserDefined();
obj = obj.regfunc()(4);
)";
