/*
    PDFStreamForResponse is an implementation of a write stream that writes directly to an HTTP response.
    Using this stream frees the user from having to create a PDF file on disk when generating on-demand PDFs
*/
function PDFStreamForResponse(inResponse)
{
    this.response = inResponse;
    this.position = 0;
}

PDFStreamForResponse.prototype.write = function(inBytesArray)
{
    if(inBytesArray.length > 0)
    {
        this.response.write(Buffer.from(inBytesArray));
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

module.exports = PDFStreamForResponse;