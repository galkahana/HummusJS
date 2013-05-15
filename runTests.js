var fs = require('fs');
var cp = require('child_process');

if (!String.prototype.endsWith) {
    Object.defineProperty(String.prototype, 'endsWith', {
                          enumerable: false,
                          configurable: false,
                          writable: false,
                          value: function (searchString, position) {
                          position = position || this.length;
                          position = position - searchString.length;
                          return this.lastIndexOf(searchString) === position;
                          }
                          });
}

var files = fs.readdirSync(__dirname + '/tests');
files.forEach(
        function(element, index, array)
              {
                    var aString = element.toString();
                    if(aString.endsWith('.js'))
                        cp.spawn('node',[aString], {cwd:__dirname + '/tests', stdio: 'inherit' });
              }
    );