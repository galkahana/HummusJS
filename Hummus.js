// start with binary objects
module.exports = require('./build/Release/Hummus');

function PDFStreamForResponse(inResponse)
{
    this.response = inResponse;
    this.position = 0;
}

PDFStreamForResponse.prototype.write = function(inBytesArray)
{
    if(inBytesArray.length > 0)
    {
        this.response.write(new Buffer(inBytesArray));
        this.position+=inBytesArray.length;
        return inBytesArray.length;
    }
    else
        return 0;
};


PDFStreamForResponse.prototype.getCurrentPosition = function()
{
    return this.position;
};

module.exports.PDFStreamForResponse = PDFStreamForResponse;
