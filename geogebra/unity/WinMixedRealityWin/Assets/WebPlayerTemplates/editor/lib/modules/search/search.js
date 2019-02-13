define(["require","exports"],function(t,e){"use strict";Object.defineProperty(e,"__esModule",{value:!0});var n=function(){function t(){}return t.prototype.init=function(t){return this.input=t,this},t.prototype.listen=function(t,e,n){if(void 0===n&&(n=this.input),!n)throw new Error("Search: No input element found.");return n.addEventListener(t,e),this},t.prototype.stopListening=function(t,e,n){return void 0===n&&(n=this.input),n.removeEventListener(t,e),this},t.prototype.trigger=function(t){var e=new Event(t,{bubbles:!0,cancelable:!0});return this.input.dispatchEvent(e),this},t.prototype.focusout=function(t){var e=this,n=function(t){e.input.blur()};return t?(t.removeEventListener("mousedown",n),t.addEventListener("mousedown",n)):this.input===document.activeElement&&this.input.blur(),this},t.prototype.reset=function(){return this.input.value="",this},t.prototype.getInput=function(){return this.input.value},t}();e.default=n});