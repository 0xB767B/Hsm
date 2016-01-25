#include <cstdio>
#include "hsm.h"

/* Implements the following state machine from Miro Samek's
 * Practical Statecharts in C/C++
 *
 * |-init-----------------------------------------------------|
 * |                           s0                             |
 * |----------------------------------------------------------|
 * |                                                          |
 * |    |-init-----------|        |-------------------------| |
 * |    |       s1       |---c--->|            s2           | |
 * |    |----------------|<--c----|-------------------------| |
 * |    |                |        |                         | |
 * |<-d-| |-init-------| |        | |-init----------------| | |
 * |    | |     s11    |<----f----| |          s21        | | |
 * | /--| |------------| |        | |---------------------| | |
 * | a  | |            | |        | |                     | | |
 * | \->| |            |------g--------->|-init------|    | | |
 * |    | |____________| |        | |-b->|    s211   |---g--->|
 * |    |----b---^       |------f------->|           |    | | |
 * |    |________________|        | |<-d-|___________|<--e----|
 * |                              | |_____________________| | |
 * |                              |_________________________| |
 * |__________________________________________________________|
 */

class TestHSM;

typedef CompState<TestHSM,0>     Top;
typedef CompState<TestHSM,1,Top>   S0;
typedef CompState<TestHSM,2,S0>      S1;
typedef LeafState<TestHSM,3,S1>        S11;
typedef CompState<TestHSM,4,S0>      S2;
typedef CompState<TestHSM,5,S2>        S21;
typedef LeafState<TestHSM,6,S21>         S211;

enum Signal { A_SIG, B_SIG, C_SIG, D_SIG, E_SIG, F_SIG, G_SIG, H_SIG };

class TestHSM;

#define HSMINIT(State, InitState) \
    template<> inline void State::init(TestHSM& h) { \
       Init<InitState> i(h); \
       printf(#State "-INIT;"); \
    }

HSMINIT(Top, S0)
HSMINIT(S0, S1)
HSMINIT(S1, S11)
HSMINIT(S2, S21)
HSMINIT(S21, S211)

class TestHSM {
public:
    TestHSM() { Top::init(*this); }
    ~TestHSM() {}
    void next(const TopState<TestHSM>& state) {
        state_ = &state;
    }
    Signal getSig() const { return sig_; }
    void dispatch(Signal sig) {
        sig_ = sig;
        state_->handler(*this);
    }
    void foo(int i) {
        foo_ = i;
    }
    int foo() const {
        return foo_;
    }
private:
    const TopState<TestHSM>* state_;
    Signal sig_;
    int foo_;
};

bool testDispatch(char c) {
    static TestHSM test;
    if (c<'a' || 'h'<c) {
        return false;
    }
    printf("\nSignal<-%c: ", c);
    test.dispatch((Signal)(c-'a'));
    printf("\n");
    return true;
}

int main(int, char**) {
    testDispatch('a');
    testDispatch('b');
    testDispatch('d');
    testDispatch('e');
    testDispatch('i');
    testDispatch('f');
    testDispatch('i');
    testDispatch('i');
    testDispatch('f');
    testDispatch('a');
    testDispatch('b');
    testDispatch('d');
    testDispatch('d');
    testDispatch('e');
    testDispatch('g');
    testDispatch('h');
    testDispatch('h');
    testDispatch('c');
    testDispatch('g');
    testDispatch('c');
    testDispatch('c');
    return 0;
}

#define HSMHANDLER(State) \
    template<> template<typename X> inline void State::handle(TestHSM& h, const X& x) const

HSMHANDLER(S0) {
    switch (h.getSig()) {
        case E_SIG: { Tran<X, This, S211> t(h);
            printf("s0-E;");
            return; }
        default:
            break;
    }
    return Base::handle(h, x);
}

HSMHANDLER(S1) {
    switch (h.getSig()) {
        case A_SIG: { Tran<X, This, S1> t(h);
            printf("s1-A;"); return; }
        case B_SIG: { Tran<X, This, S11> t(h);
            printf("s1-B;"); return; }
        case C_SIG: { Tran<X, This, S2> t(h);
            printf("s1-C;"); return; }
        case D_SIG: { Tran<X, This, S0> t(h);
            printf("s1-D;"); return; }
        case F_SIG: { Tran<X, This, S211> t(h);
            printf("s1-F;"); return; }
        default: break;
    }
    return Base::handle(h, x);
}

HSMHANDLER(S11) {
    switch (h.getSig()) {
        case G_SIG: { Tran<X, This, S211> t(h);
            printf("s11-G;"); return; }
        case H_SIG: if (h.foo()) {
                printf("s11-H");
                h.foo(0); return;
            } break;
        default: break;
    }
    return Base::handle(h, x);
}

HSMHANDLER(S2) {
    switch (h.getSig()) {
        case C_SIG: { Tran<X, This, S1> t(h);
            printf("s2-C"); return; }
        case F_SIG: { Tran<X, This, S11> t(h);
            printf("s2-F"); return; }
        default: break;
    }
    return Base::handle(h, x);
}

HSMHANDLER(S21) {
    switch (h.getSig()) {
        case B_SIG: { Tran<X, This, S211> t(h);
            printf("s21-B;"); return; }
        case H_SIG: if (!h.foo()) {
                Tran<X, This, S21> t(h);
                printf("s21-H;"); h.foo(1);
                return;
            } break;
        default: break;
    }
    return Base::handle(h, x);
}

HSMHANDLER(S211) {
    switch (h.getSig()) {
        case D_SIG: { Tran<X, This, S21> t(h);
            printf("s211-D;"); return; }
        case G_SIG: { Tran<X, This, S0> t(h);
            printf("s211-G;"); return; }
        default: break;
    }
    return Base::handle(h, x);
}

#define HSMENTRY(State) \
    template<> inline void State::entry(TestHSM&) { \
        printf(#State "-ENTRY;"); \
    }

HSMENTRY(S0)
HSMENTRY(S1)
HSMENTRY(S11)
HSMENTRY(S2)
HSMENTRY(S21)
HSMENTRY(S211)

#define HSMEXIT(State) \
    template<> inline void State::exit(TestHSM&) { \
        printf(#State "-EXIT;"); \
    }

HSMEXIT(S0)
HSMEXIT(S1)
HSMEXIT(S11)
HSMEXIT(S2)
HSMEXIT(S21)
HSMEXIT(S211)
