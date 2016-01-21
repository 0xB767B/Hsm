#ifndef HSM_HPP
#define HSM_HPP

// This code is from:
// Yet Another Hierarchical State Machine
// by Stefan Heinzmann
// Overload issue 64 december 2004
// http://www.state-machine.com/resources/Heinzmann04.pdf

/* This is a basic implementation of UML Statecharts.
 * The key observation is that the machine can only
 * be in a leaf state at any given time. The composite
 * states are only traversed, never final.
 * Only the leaf states are ever instantiated. The composite
 * states are only mechanisms used to generate code. They are
 * never instantiated.
 */

// Helpers

// A gadget from Herb Sutter's GotW #71 -- depends on SFINAE
template<class D, class B>
class IsDerivedFrom {
    class Yes { char a[1]; };
    class No  { char a[10]; };
    static Yes Test(B*); // undefined
    static No Test(...); // undefined
public:
    enum { Res = sizeof(Test(static_cast<D*>(0))) == sizeof(Yes) ? 1 : 0 };
};

template<bool> class Bool {};

// Top State, Composite State and Leaf State

template <typename H>
struct TopState {
    typedef H Host;
    typedef void Base;
    virtual void handler(Host&) const = 0;
    virtual unsigned getId() const = 0;
};

template <typename H, unsigned id, typename B>
struct CompState;

template <typename H, unsigned id, typename B = CompState<H, 0, TopState<H> > >
struct CompState : B {
    typedef B Base;
    typedef CompState<H, id, Base> This;
    template <typename X> void handle(H& h, const X& x) const { Base::handle(h, x); }
    static void init(H&); // no implementation
    static void entry(H&) {}
    static void exit(H&) {}
};

template <typename H>
struct CompState<H, 0, TopState<H> > : TopState<H> {
    typedef TopState<H> Base;
    typedef CompState<H, 0, Base> This;
    template <typename X> void handle(H&, const X&) const {}
    static void init(H&); // no implementation
    static void entry(H&) {}
    static void exit(H&) {}
};

template <typename H, unsigned id, typename B = CompState<H, 0, TopState<H> > >
struct LeafState : B {
    typedef H Host;
    typedef B Base;
    typedef LeafState<H, id, Base> This;
    template <typename X> void handle(H& h, const X& x) const { Base::handle(h, x); }
    virtual void handler(H& h) const { handle(h, *this); }
    virtual unsigned getId() const { return id; }
    static void init(H& h) { h.next(obj); } // don't specialize this
    static void entry(H&) {}
    static void exit(H&) {}
    static const LeafState obj; // only the leaf states have instances
};

template <typename H, unsigned id, typename B>
const LeafState<H, id, B> LeafState<H, id, B>::obj;

// Transition Object

template <typename C, typename S, typename T>
// Current, Source, Target
struct Tran {
    typedef typename C::Host Host;
    typedef typename C::Base CurrentBase;
    typedef typename S::Base SourceBase;
    typedef typename T::Base TargetBase;
    enum { // work out when to terminate template recursion
        eTB_CB = IsDerivedFrom<TargetBase, CurrentBase>::Res,
        eS_CB = IsDerivedFrom<S, CurrentBase>::Res,
        eS_C = IsDerivedFrom<S, C>::Res,
        eC_S = IsDerivedFrom<C, S>::Res,
        exitStop = eTB_CB && eS_C,
        entryStop = eS_C || (eS_CB && !eC_S)
    };
    // We use overloading to stop recursion.
    // The more natural template specialization
    // method would require to specialize the inner
    // template without specializing the outer one,
    // which is forbidden.
    static void exitActions(Host&, Bool<true>) {}
    static void exitActions(Host&h, Bool<false>) {
        C::exit(h);
        Tran<CurrentBase, S, T>::exitActions(h, Bool<exitStop>());
    }
    static void entryActions(Host&, Bool<true>) {}
    static void entryActions(Host& h, Bool<false>) {
        Tran<CurrentBase, S, T>::entryActions(h, Bool<entryStop>());
        C::entry(h);
    }
    Tran(Host & h) : host_(h) {
        exitActions(host_, Bool<false>());
    }
    ~Tran() {
        Tran<T, S, T>::entryActions(host_, Bool<false>());
        T::init(host_);
    }
    Host& host_;
};

// Initializer for Compound States

template <typename T>
struct Init {
    typedef typename T::Host Host;
    Init(Host& h) : host_(h) {}
    ~Init() {
        T::entry(host_);
        T::init(host_);
    }
    Host& host_;
};

#endif // HSM_HPP

