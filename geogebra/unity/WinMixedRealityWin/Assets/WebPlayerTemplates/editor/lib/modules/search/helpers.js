define(["require","exports"],function(e,t){"use strict";Object.defineProperty(t,"__esModule",{value:!0}),t.assetSearch=function(e,t){var r=t,i=e.target.value;return""!==i&&(r=t.filter(function(e){var t=e.querySelector("[data-link]");return!!t&&-1!==t.getAttribute("data-link").toLowerCase().indexOf(i.toLowerCase())})),r.forEach(function(e,t){e.style.top=34*t+"px",e.setAttribute("vList-index",String(t))}),r}});