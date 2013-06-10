var hummus = require('../hummus');



{
    var pdfWriterA = hummus.createWriter('./output/SimpleContentShutdownRestart.PDF');
    var page = pdfWriterA.createPage(0,0,595,842);
    var font = pdfWriterA.getFontForFile('./TestMaterials/fonts/arial.ttf');
    
    pdfWriterA.startPageContentContext(page).q()
                                            .k(100,0,0,0)
                                            .re(100,500,100,100)
                                            .f()
                                            .Q()
                                            .q()
                                            .k(0,100,100,0)
                                            .re(200,600,200,100)
                                            .f()
                                            .Q()
                                            .q()
                                            .G(0.5)
                                            .w(3)
                                            .m(200,600)
                                            .l(400,400)
                                            .S()
                                            .Q()
                                            .BT()
                                            .k(0,0,0,1)
                                            .Tf(font,1)
                                            .Tm(30,0,0,30,78.4252,662.8997)
                                            .Tj('hello world')
                                            .ET();
                                            
    pdfWriterA.writePage(page)
             .shutdown('./output/ShutDownRestartState.txt');
}

{
    var pdfWriterB = hummus.createWriterToContinue('./output/SimpleContentShutdownRestart.PDF',
                                                    './output/ShutDownRestartState.txt');
    var page = pdfWriterB.createPage(0,0,595,842);
    
    
    var contentContext = pdfWriterB.startPageContentContext(page).q()
                                                                 .k(100,0,0,0)
                                                                 .re(100,500,100,100)
                                                                 .f()
                                                                 .Q();
    pdfWriterB.pausePageContentContext(contentContext);

    var xobjectForm = pdfWriterB.createFormXObject(0,0,200,100);
    xobjectForm.getContentContext().q()
                                    .k(0,100,100,0)
                                    .re(0,0,200,100)
                                    .f()
                                    .Q();
    pdfWriterB.endFormXObject(xobjectForm);
    
    contentContext.q()
                    .cm(1,0,0,1,200,600)
                    .doXObject(xobjectForm)
                    .Q()
                    .q()
                    .G(0.5)
                    .w(3)
                    .m(200,600)
                    .l(400,400)
                    .S()
                    .Q()
                    .q()
                    .cm(1,0,0,1,200,200)
                    .doXObject(xobjectForm)
                    .Q();
    
    pdfWriterB.writePage(page)
                .end();

}

console.log('done - ok');


	
