using CsvHelper;
using CsvHelper.Configuration;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSVIngester
{
    public class InventoryRunDescriptor
    {
        public string ebay_id { get; set; }
        public string asin { get; set; }
        public string vat { get; set; }
        public InventoryRunDescriptor()
        {
            vat = "";
        }
        public InventoryRunDescriptor(string ebay_id_p, string asin_p, double vat_p)
        {
            ebay_id = ebay_id_p;
            asin = asin_p;
            if (vat_p == GlobalVariables.NULLValue)
                vat = "";
            else
                vat = vat_p.ToString();
        }
    }
    public class WriteCSVFile
    {
        StreamWriter writer;
        CsvWriter csv;
        string FileName;
        int RowsAdded = 0;
        public void CreateInventoryRunFile(string FileName_p)
        {
            System.IO.Directory.CreateDirectory("./reports");
            FileName = FileName_p;
            //delete any previous file
            File.Delete(FileName);
            //create a new csv file
            writer = new StreamWriter(FileName);
            csv = new CsvWriter(writer);
            csv.WriteHeader< InventoryRunDescriptor>();
            csv.NextRecord();
        }
        public void InventoryRunFileAddRow(string ebay_id,string asin, double vat = -1)
        {
            csv.WriteRecord<InventoryRunDescriptor>(new InventoryRunDescriptor(ebay_id, asin, vat));
            csv.NextRecord();
        }
        public void CreateInventoryExportFile()
        {
            File.Delete(FileName);
            //create a new csv file
            writer = new StreamWriter(FileName);
            csv = new CsvWriter(writer);
            csv.WriteHeader<InventoryRunDescriptor>();
            csv.NextRecord();
        }
        public void InventoryExportFileAddRow(string ebay_id, string asin, double vat)
        {
            csv.WriteRecord<InventoryRunDescriptor>(new InventoryRunDescriptor(ebay_id, asin, vat));
            csv.NextRecord();
            RowsAdded++;
        }
        public void Dispose()
        {
            csv.Flush();
            writer.Flush();
            csv.Dispose();
            writer.Dispose();
        }
        ~WriteCSVFile()
        {
        }
        public void CreateDynamicFile(string FileName_p)
        {
            System.IO.Directory.CreateDirectory("./reports");
            FileName = FileName_p;
            //delete any previous file
            File.Delete(FileName);
            //create a new csv file
            writer = new StreamWriter(FileName);
            csv = new CsvWriter(writer, new Configuration { HasHeaderRecord = false });
        }
        public void WriteDynamicFileRow(dynamic RowData)
        {
            RowsAdded++;
            csv.WriteRecord<dynamic>(RowData);
            csv.NextRecord();
        }
        public void WriteDynamicFileHeader(dynamic RowData)
        {
            RowsAdded++;
            csv.WriteDynamicHeader(RowData);
            csv.NextRecord();
        }
        public int RowsWritten()
        {
            return RowsAdded;
        }
    }
}
