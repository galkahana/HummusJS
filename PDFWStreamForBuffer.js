function PDFWStreamForBuffer()
{
    this.buffer = null;
    this.position = 0;
}

PDFWStreamForBuffer.prototype.write = function(inBytesArray)
{
    if(inBytesArray.length > 0)
    {
        if(!this.buffer)
        {
            this.buffer = Buffer.from(inBytesArray);
        }
        else
        {
            this.buffer = Buffer.concat([this.buffer, Buffer.from(inBytesArray)]);
        }

        this.position += inBytesArray.length;
        return inBytesArray.length;
    }

    return 0;
};

PDFWStreamForBuffer.prototype.getCurrentPosition = function()
{
    return this.position;
};

module.exports = PDFWStreamForBuffer;
