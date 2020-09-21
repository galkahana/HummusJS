var fs = require('fs');
/*
    PDFRStreamForFile is an implementation of a read stream using the supplied file path.
*/

function PDFRStreamForFile(inPath)
{
    this.rs = fs.openSync(inPath,'r');
    this.path = inPath;
    this.rposition = 0;
    this.fileSize = fs.statSync(inPath)["size"];
}

PDFRStreamForFile.prototype.read = function(inAmount)
{
    var buffer = Buffer.alloc(inAmount*2);
    var bytesRead = fs.readSync(this.rs, buffer, 0, inAmount,this.rposition);
    var arr = [];

    for(var i=0;i<bytesRead;++i)
        arr.push(buffer[i]);
    this.rposition+=bytesRead;
    return arr;
}

PDFRStreamForFile.prototype.notEnded = function()
{
    return this.rposition < this.fileSize;
}

PDFRStreamForFile.prototype.setPosition = function(inPosition)
{
    this.rposition = inPosition;
}

PDFRStreamForFile.prototype.setPositionFromEnd = function(inPosition)
{
    this.rposition = this.fileSize-inPosition;
}

PDFRStreamForFile.prototype.skip = function(inAmount)
{
    this.rposition += inAmount;
}

PDFRStreamForFile.prototype.getCurrentPosition = function()
{
    return this.rposition;
}

function noop() {}

PDFRStreamForFile.prototype.close = function(inCallback)
{
    fs.close(this.rs,inCallback ? inCallback : noop);
};

module.exports = PDFRStreamForFile;