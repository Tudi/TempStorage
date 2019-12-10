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
    public class AmazonOrdersRowDescriptor
    {
        public string date { get; set; }
        public string ORDER_ID { get; set; }
        public string TITLE { get; set; }
        public string GROSS { get; set; }
        public string vat { get; set; }
        public string NAME { get; set; }
        public string ADDRESS { get; set; }
        public string asin { get; set; }
        public string NET { get; set; }
        public string vat_rate { get; set; }
        public AmazonOrdersRowDescriptor(string datep, string ORDER_IDp, string TITLEp, float GROSSp, float vatp, string BuyerNamep, string Buyeraddrp, string asinp, float NETp, float vat_ratep)
        {
            date = datep;
            ORDER_ID = ORDER_IDp;
            TITLE = TITLEp;
            GROSS = GROSSp.ToString();
            vat = vatp.ToString();
            NAME = BuyerNamep;
            ADDRESS = Buyeraddrp;
            asin = asinp;
            if (NETp >= 0)
                NET = NETp.ToString();
            else
                NET = "";
            if (vat_ratep >= 0)
                vat_rate = vat_ratep.ToString();
            else
                vat_rate = "";
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
        public void CreateAmazonOrdersFile(string FileName_p)
        {
            FileName = FileName_p;
            //delete any previous file
            File.Delete(FileName);
            //create a new csv file
            writer = new StreamWriter(FileName);
            csv = new CsvWriter(writer);
            csv.WriteHeader<AmazonOrdersRowDescriptor>();
            csv.NextRecord();
        }
        public void AmazonOrdersExportFileAddRow(string datecol, string ORDER_ID, string TITLE, float GROSS, float vat, string BuyerName, string Buyeraddr, string asin, float NET, float vat_rate)
        {
            csv.WriteRecord<AmazonOrdersRowDescriptor>(new AmazonOrdersRowDescriptor(datecol, ORDER_ID, TITLE, GROSS, vat, BuyerName, Buyeraddr, asin, NET, vat_rate));
            csv.NextRecord();
        }
    }
}
