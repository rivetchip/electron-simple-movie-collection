'use strict';

import {Component} from 'preact';

// Bind an object/factory of actions to the store and wrap them.
export function mapActions(actions, store) {
    if (typeof actions==='function') actions = actions(store);
    let mapped = {};
    for (let i in actions) {
        mapped[i] = store.action(actions[i]);
    }
    return mapped;
}

// Lighter Object.assign stand-in
export function assign(obj, props) {
    for (let i in props) obj[i] = props[i];
    return obj;
}

/**
 * Creates a new store, which is a tiny evented state container.
 * @name createStore
 * @param {Object} [state={}]		Optional initial state
 * @returns {store}
 * @example
 * let store = createStore();
 * store.subscribe( state => console.log(state) );
 * store.setState({ a: 'b' });   // logs { a: 'b' }
 * store.setState({ c: 'd' });   // logs { a: 'b', c: 'd' }
 */
export function createStore(state) {
    let listeners = [];

    function unsubscribe(listener) {
        let out = [];
        for (let i=0; i<listeners.length; i++) {
            if (listeners[i]===listener) {
                listener = null;
            }
            else {
                out.push(listeners[i]);
            }
        }
        listeners = out;
    }

    function setState(update, overwrite, action) {
        state = overwrite ? update : assign(assign({}, state), update);
        let currentListeners = listeners;
        for (let i=0; i<currentListeners.length; i++) currentListeners[i](state, action);
    }

    /**
     * An observable state container, returned from {@link createStore}
     * @name store
     */

    return /** @lends store */ {

        /**
         * Create a bound copy of the given action function.
         * The bound returned function invokes action() and persists the result back to the store.
         * If the return value of `action` is a Promise, the resolved value will be used as state.
         * @param {Function} action	An action of the form `action(state, ...args) -> stateUpdate`
         * @returns {Function} boundAction()
         */
        action(action) {
            function apply(result) {
                setState(result, false, action);
            }

            // Note: perf tests verifying this implementation: https://esbench.com/bench/5a295e6299634800a0349500
            return function() {
                let args = [state];
                for (let i=0; i<arguments.length; i++) args.push(arguments[i]);
                let ret = action.apply(this, args);
                if (ret!=null) {
                    if (ret.then) return ret.then(apply);
                    return apply(ret);
                }
            };
        },

        /**
         * Apply a partial state object to the current state, invoking registered listeners.
         * @param {Object} update				An object with properties to be merged into state
         * @param {Boolean} [overwrite=false]	If `true`, update will replace state instead of being merged into it
         */
        setState,

        /**
         * Register a listener function to be called whenever state is changed. Returns an `unsubscribe()` function.
         * @param {Function} listener	A function to call when state changes. Gets passed the new state.
         * @returns {Function} unsubscribe()
         */
        subscribe(listener) {
            listeners.push(listener);
            return () => { unsubscribe(listener); };
        },

        /**
         * Remove a previously-registered listener function.
         * @param {Function} listener	The callback previously passed to `subscribe()` that should be removed.
         * @function
         */
        unsubscribe,

        /**
         * Retrieve the current state object.
         * @returns {Object} state
         */
        getState() {
            return state;
        }
    };
}








/**
 * Wire a component up to the store. Passes state as props, re-renders on change.
 */
export function connect(actions, ChildCallback) {
    function Wrapper(props, context) {
        const self = this;
        const store = context.store;

        const attributes = {
            props,
            state: (store && store.getState()) || {},
            actions: (actions && mapActions(actions, store)) || {}
        };
        function update(state, action) {
            attributes.state = state;
            return self.setState({}); // trigger component
        }
        this.componentDidMount = function() {
            store.subscribe(update);
        };
        this.componentWillUnmount = function() {
            store.unsubscribe(update);
        };
        this.render = function(props) {
            return ChildCallback(attributes.state, attributes.actions, attributes.props);
        };
    }
    return (Wrapper.prototype = new Component()).constructor = Wrapper;
}


/**
 * Provider exposes a store (passed as `props.store`) into context.
 *
 * Generally, an entire application is wrapped in a single `<Provider>` at the root.
 * @class
 * @extends Component
 * @param {Object} props
 * @param {Store} props.store	A {Store} instance to expose via context.
 */
export function Provider(props) {
    this.getChildContext = function() {
        return { store: props.store };
    };
}
Provider.prototype.render = function(props) {
    return props.children && props.children[0] || props.children;;
};
