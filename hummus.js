var fs = require('fs');
var path = require('path');
var pregyp = require('@mapbox/node-pre-gyp');
var binding_path = pregyp.find(path.resolve(path.join(__dirname,'./package.json')));
var hummus = module.exports = require(binding_path);
var EventEmitter = require('events').EventEmitter;

/*
    addons to PDFWriter prototype for events listening
*/

hummus.PDFWriter.prototype.getEvents = function() {
    if(!this.events)
        this.events = new EventEmitter();
    return this.events;
}

hummus.PDFWriter.prototype.triggerDocumentExtensionEvent = function(eventName,eventParams) {
    eventParams.writer = this;
    this.getEvents().emit(eventName,eventParams);
}


/*
    addon class for simple node wrappers
*/
hummus.PDFStreamForResponse = require('./PDFStreamForResponse');
hummus.PDFWStreamForFile = require('./PDFWStreamForFile');
hummus.PDFRStreamForFile = require('./PDFRStreamForFile');
hummus.PDFRStreamForBuffer = require('./PDFRStreamForBuffer');
hummus.PDFWStreamForBuffer = require('./PDFWStreamForBuffer');
