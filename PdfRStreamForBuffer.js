/*
    PDFRStreamForBuffer is an implementation of a read stream using a supplied array

    @author Luciano Júnior
*/

function PDFRStreamForBuffer(buffer){
    this.innerBuffer = buffer;
    this.rposition = 0;
    this.fileSize = buffer.byteLength;
}

PDFRStreamForBuffer.prototype.read = function(inAmount){
    const previousPosition = this.rposition;
    this.rposition += inAmount;
    return [].concat(this.innerBuffer.slice(previousPosition, this.rposition));
}

PDFRStreamForBuffer.prototype.notEnded = function(){
    return this.rposition < this.fileSize;
}

PDFRStreamForBuffer.prototype.setPosition = function(inPosition){
    this.rposition = inPosition;
}

PDFRStreamForBuffer.prototype.setPositionFromEnd = function(inPosition){
    this.rposition = this.fileSize-inPosition;
}

PDFRStreamForBuffer.prototype.skip = function(inAmount){
    this.rposition += inAmount;
}

PDFRStreamForBuffer.prototype.getCurrentPosition = function(){
    return this.rposition;
}


module.exports = PDFRStreamForBuffer;
