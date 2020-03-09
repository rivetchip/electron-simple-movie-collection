'use strict';

import {h} from 'preact';


export function Header(props, childrens) {

    return <div id="header">
        <div class="controls">
            <button id="menu" onclick={props.actionOpenMenu}>Menu</button>
        </div>
        <div id="title">
            xx
        </div>
    </div>
}


