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
            ImportResultCSV.CreateInventoryRunFile();
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
                    DBHandler.InvenotryInsertResultCodes ret = GlobalVariables.DBStorage.InsertInventory(Ebay_id, asin, "0");
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
    }
}
