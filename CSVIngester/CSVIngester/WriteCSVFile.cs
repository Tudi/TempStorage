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
        public InventoryRunDescriptor(string ebay_id_p, string asin_p, float vat_p)
        {
            ebay_id = ebay_id_p;
            asin = asin_p;
            if (vat_p < 0)
                vat = "";
            else
                vat = vat_p.ToString();
        }
    }
    public class InventoryRunDescriptorMap : ClassMap<InventoryRunDescriptor>
    {
        public InventoryRunDescriptorMap()
        {
            Map(m => m.ebay_id).Index(0).Name("ebay_id");
            Map(m => m.asin).Index(1).Name("asin");
        }
    }
    public class WriteCSVFile
    {
        StreamWriter writer;
        CsvWriter csv;
        string FileName;
        public void CreateInventoryRunFile(string FileName_p = "")
        {
            FileName = "INVENTORY-RUN.csv";
            if (FileName_p.Length != 0)
                FileName = FileName_p;
            //delete any previous file
            File.Delete(FileName);
            //create a new csv file
            writer = new StreamWriter(FileName);
            csv = new CsvWriter(writer);
            csv.WriteHeader< InventoryRunDescriptor>();
            csv.NextRecord();
        }
        public void InventoryRunFileAddRow(string ebay_id,string asin, float vat = -1)
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
        public void InventoryExportFileAddRow(string ebay_id, string asin, float vat)
        {
            csv.WriteRecord<InventoryRunDescriptor>(new InventoryRunDescriptor(ebay_id, asin, vat));
            csv.NextRecord();
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
    }
}
