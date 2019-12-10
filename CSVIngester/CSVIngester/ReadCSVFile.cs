using CsvHelper;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CSVIngester
{
    public class ReadCSVFile
    {
        public static bool CheckHeaderMatch(string[] FileColumnNames, string[] ExpectedColumnames)
        {
            //all values must mutch. Both count, value and order
            if (FileColumnNames.Length != ExpectedColumnames.Length)
                return false;
            int Order = 0;
            foreach (var itr in ExpectedColumnames)
            {
                if (FileColumnNames[Order].ToLower() != itr.ToLower())
                    return false;
                Order++;
            }
            return true;
        }
        public static string GetMatchingColumnName(string[] FileColumnNames, string ExpectedColumnName)
        {
            foreach (var itr2 in FileColumnNames)
                if (ExpectedColumnName.ToLower() == itr2.ToLower())
                    return itr2;
            return "";
        }
        public static bool CheckHeaderHasColumns(string[] FileColumnNames, string[] ExpectedColumnames)
        {
            int FoundCount = 0;
            foreach (var itr in ExpectedColumnames)
                foreach (var itr2 in FileColumnNames)
                    if (itr.ToLower() == itr2.ToLower())
                        FoundCount++;
            return ExpectedColumnames.Length == FoundCount;
        }
        public static void ReadInvenotryCSVFile(string FileName)
        {
            if (GlobalVariables.ImportingToDBBlock == true)
            {
                GlobalVariables.Logger.Log("Another thread is already importing. Please wait until it finishes");
                return;
            }
            GlobalVariables.ImportingToDBBlock = true;

            GlobalVariables.Logger.Log("File import destination database is 'inventory'");
            WriteCSVFile ImportResultCSV = new WriteCSVFile();
            ImportResultCSV.CreateInventoryRunFile("./reports/INVENTORY-RUN.csv");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string EbayIdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ebay_id");
                string AsinColName = GetMatchingColumnName(csv.Context.HeaderRecord, "asin");
                if(EbayIdColName.Length == 0 || AsinColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Not an Inventory csv file : " + FileName);
                    GlobalVariables.ImportingToDBBlock = false;
                    return;
                }
                int RowsRead = 0;
                int RowsInserted = 0;
                int RowsUpdated = 0;
                int RowsSkipped = 0;
                while (csv.Read())
                {
                    string Ebay_id = csv.GetField<string>(EbayIdColName);
                    string asin = csv.GetField<string>(AsinColName);

                    //check if the read data is correct
                    if(Ebay_id == null || Ebay_id.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " ebay_id does not have a value");
                    if (asin == null || asin.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " asin does not have a value");

                    //try to add the new row to the DB
                    DBHandler.InvenotryInsertResultCodes ret = GlobalVariables.DBStorage.InsertInventory(Ebay_id, asin, "-1");
                    if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew)
                        RowsInserted++;
                    else if (ret == DBHandler.InvenotryInsertResultCodes.RowExistedButWasEmpty)
                        RowsUpdated++;
                    else
                        RowsSkipped++;
                    RowsRead++;
                    //add to the import log
                    if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew || ret == DBHandler.InvenotryInsertResultCodes.RowExistedButWasEmpty)
                        ImportResultCSV.InventoryRunFileAddRow(Ebay_id, asin);
//if (RowsRead == 15)    break;
                }
                GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
                GlobalVariables.Logger.Log("CSV file rows inserted : " + RowsInserted);
                GlobalVariables.Logger.Log("CSV file rows updated : " + RowsUpdated);
                GlobalVariables.Logger.Log("CSV file rows skipped : " + RowsSkipped);
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
            ImportResultCSV.Dispose();
            GlobalVariables.ImportingToDBBlock = false;
        }

        public static void ReadEbayVATCSVFile(string FileName)
        {
            GlobalVariables.Logger.Log("Ebay VAT File import destination database is 'inventory'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvHelper.CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string EbayIdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ebay_id");
                string VatColName = GetMatchingColumnName(csv.Context.HeaderRecord, "vat_rate");
                if (EbayIdColName.Length == 0 || VatColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Not an VAT csv file : " + FileName);
                    GlobalVariables.ImportingToDBBlock = false;
                    return;
                }
                int RowsRead = 0;
                while (csv.Read())
                {
                    string Ebay_id = csv.GetField<string>(EbayIdColName);
                    string vat = csv.GetField<string>(VatColName);

                    //check if the read data is correct
                    if (Ebay_id == null || Ebay_id.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " ebay_id does not have a value");
                    if (vat == null || vat.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " vat does not have a value");

                    GlobalVariables.DBStorage.UpdateInventoryVatEbay(Ebay_id, vat);
//if (RowsRead == 15) break;
                    RowsRead++;
                }
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }

        public static void ReadAsinVATCSVFile(string FileName)
        {
            GlobalVariables.Logger.Log("Asin VAT File import destination database is 'inventory'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvHelper.CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string AsinColName = GetMatchingColumnName(csv.Context.HeaderRecord, "asin");
                string VatColName = GetMatchingColumnName(csv.Context.HeaderRecord, "vat_rate");
                if (AsinColName.Length == 0 || VatColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Not an VAT csv file : " + FileName);
                    GlobalVariables.ImportingToDBBlock = false;
                    return;
                }
                int RowsRead = 0;
                while (csv.Read())
                {
                    string asin = csv.GetField<string>(AsinColName);
                    string vat = csv.GetField<string>(VatColName);

                    //check if the read data is correct
                    if (asin == null || asin.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " asin does not have a value");
                    if (vat == null || vat.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " vat does not have a value");

                    GlobalVariables.DBStorage.UpdateInventoryVatAsin(asin, vat);
//if (RowsRead == 15) break;
                    RowsRead++;
                }
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }

        public static void ReadVATCSVFile(string FileName)
        {
            if (GlobalVariables.ImportingToDBBlock == true)
            {
                GlobalVariables.Logger.Log("Another thread is already importing. Please wait until it finishes");
                return;
            }
            GlobalVariables.ImportingToDBBlock = true;

            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvHelper.CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string[] ExpectedColumnames1 = { "ebay_id", "vat_rate" };
                string[] ExpectedColumnames2 = { "asin", "vat_rate" };

                if ( CheckHeaderHasColumns( csv.Context.HeaderRecord, ExpectedColumnames1) )
                    ReadEbayVATCSVFile(FileName);
                else if (CheckHeaderHasColumns(csv.Context.HeaderRecord, ExpectedColumnames2))
                    ReadAsinVATCSVFile(FileName);
                else
                {
                    GlobalVariables.Logger.Log("Not a VAT csv file : " + FileName);
                    GlobalVariables.ImportingToDBBlock = false;
                    return;
                }
            }

            GlobalVariables.ImportingToDBBlock = false;
        }

        public static void ReadAmazonOrdersCSVFile(string FileName)
        {
            if (GlobalVariables.ImportingToDBBlock == true)
            {
                GlobalVariables.Logger.Log("Another thread is already importing. Please wait until it finishes");
                return;
            }
            GlobalVariables.ImportingToDBBlock = true;

            WriteCSVFile ImportResultCSV = new WriteCSVFile();
            ImportResultCSV.CreateAmazonOrdersFile("./reports/AMAZON-ORDERS-RUN.csv");
            GlobalVariables.Logger.Log("File import destination database is 'AMAZON-ORDERS'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string DateColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Date");
                string IdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Id");
                string TitleColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Title");
                string PriceColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Price");
                string VATColName = GetMatchingColumnName(csv.Context.HeaderRecord, "VAT");
                string BuyerColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Buyer");
                string AddressColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Address");
                string ASINColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ASIN");
                if (AddressColName.Length == 0 || ASINColName.Length == 0 || DateColName.Length == 0 || IdColName.Length == 0 || TitleColName.Length == 0 || PriceColName.Length == 0 || VATColName.Length == 0 || BuyerColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Not an AMAZON-ORDERS csv file : " + FileName);
                    GlobalVariables.ImportingToDBBlock = false;
                    return;
                }
                int RowsRead = 0;
                int RowsInserted = 0;
                int RowsUpdated = 0;
                int RowsSkipped = 0;
                while (csv.Read())
                {
                    string DateCol = csv.GetField<string>(DateColName);
                    string IdCol = csv.GetField<string>(IdColName);
                    string TitleCol = csv.GetField<string>(TitleColName);
                    string PriceCol = csv.GetField<string>(PriceColName);
                    string VATCol = csv.GetField<string>(VATColName);
                    string BuyerCol = csv.GetField<string>(BuyerColName);
                    string AddressCol = csv.GetField<string>(AddressColName);
                    string ASINCol = csv.GetField<string>(ASINColName);

                    //check if the read data is correct
                    if (DateCol == null || DateCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Date does not have a value");
                    if (IdCol == null || IdCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Id does not have a value");
                    if (TitleCol == null || TitleCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Title does not have a value");
                    if (PriceCol == null || PriceCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Price does not have a value");
                    if (VATCol == null || VATCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " VAT does not have a value");
                    if (BuyerCol == null || BuyerCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Buyer does not have a value");
                    if (AddressCol == null || AddressCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Address does not have a value");
                    if (ASINCol == null || ASINCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " ASIN does not have a value");

                    float NET = float.Parse(PriceCol) - float.Parse(VATCol);
                    float VAT_RATE = float.Parse(VATCol) * 100 / NET;
                    //try to add the new row to the DB
                    DBHandler.InvenotryInsertResultCodes ret = GlobalVariables.DBStorage.InsertAmazonOrder(DateCol, IdCol, TitleCol, PriceCol, VATCol, BuyerCol, AddressCol, ASINCol, NET, VAT_RATE);
                    if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew)
                    {
                        RowsInserted++;
                        ImportResultCSV.AmazonOrdersExportFileAddRow(DateCol, IdCol, TitleCol, float.Parse(PriceCol), float.Parse(VATCol), BuyerCol, AddressCol, ASINCol, NET, VAT_RATE);
                    }
                    else
                        RowsSkipped++;
                    RowsRead++;
                }
                GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
                GlobalVariables.Logger.Log("CSV file rows inserted : " + RowsInserted);
                GlobalVariables.Logger.Log("CSV file rows updated : " + RowsUpdated);
                GlobalVariables.Logger.Log("CSV file rows skipped : " + RowsSkipped);
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
            GlobalVariables.ImportingToDBBlock = false;
        }
    }
}
