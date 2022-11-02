var hummus = require('../hummus');
const { expect } = require("chai");

describe("SigSegv test", function () {
  it("should read fields correctly", function () {
    expect(() =>
        hummus.createReader(
        __dirname + "/TestMaterials/BrokenPdfBadHeader.txt"
      )
    ).to.throw();
  });
});