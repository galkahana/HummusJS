var hummus = require('../hummus');
var emptyFileName = __dirname + '/output/sampleEmptyFileForCopying.pdf';

describe('CopyingAndMergingEmptyPage', function() {
	before(function() {
		// prepares sample empty pdf
		var pdfWriter = hummus.createWriter(emptyFileName);
		pdfWriter.writePage(pdfWriter.createPage(0,0,595,842)).end();
	});

	it('should be able to create from from empty page', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/CreateFormFromEmptyPage.pdf');
		var formIDs = pdfWriter.createFormXObjectsFromPDF(emptyFileName,hummus.ePDFPageBoxMediaBox);
		var page = pdfWriter.createPage(0,0,595,842);

		pdfWriter.startPageContentContext(page)
			.q()
			.cm(0.5,0,0,0.5,0,421)
			.doXObject(page.getResourcesDictionary().addFormXObjectMapping(formIDs[0]))
			.Q()
			.G(0)
			.w(1)
			.re(0,421,297.5,421)
			.S();

		pdfWriter.writePage(page).end();
	});

	it('should be able to create page from empty page', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/CreatePageFromEmptyPage.pdf');
		pdfWriter.appendPDFPagesFromPDF(emptyFileName);
		pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/XObjectContent.PDF');
		pdfWriter.end();
	});

	it('should be able to merge empty page to page', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/MergeEmptyPageToPage.pdf');
		var page = pdfWriter.createPage(0,0,595,842);
		var font = pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/arial.ttf');

		var contentContext = pdfWriter.startPageContentContext(page)
			.BT()
			.k(0,0,0,1)
			.Tf(font,30)
			.Tm(1,0,0,1,10,600)
			.Tj("Testing file merge")
			.ET()
			.q()
			.cm(0.5,0,0,0.5,0,0);

		pdfWriter.mergePDFPagesToPage(page,emptyFileName,{type:hummus.eRangeTypeSpecific,specificRanges:[[0,0]]});

		contentContext.Q()
			.q()
			.cm(1,0,0,1,30,500)
			.k(0,100,100,0)
			.re(0,0,200,100)
			.f()
			.Q();

		pdfWriter.writePage(page).end();
	});

	it('should be able to merge empty page to form', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/MergeEmptyPageToForm.pdf');
		var page = pdfWriter.createPage(0,0,595,842);
		var copyingContext = pdfWriter.createPDFCopyingContext(emptyFileName);
		var aForm = pdfWriter.createFormXObject(0,0,297.5,842);

		aForm.getContentContext().q().cm(0.5,0,0,0.5,0,421);
		copyingContext.mergePDFPageToFormXObject(aForm,0);
		aForm.getContentContext().Q();
		pdfWriter.endFormXObject(aForm);

		var formName = page.getResourcesDictionary().addFormXObjectMapping(aForm.id);

		pdfWriter.startPageContentContext(page).q()
			.doXObject(formName)
			.cm(1,0,0,1,297.5,0)
			.doXObject(formName)
			.Q();

		pdfWriter.writePage(page).end();
	});
});
