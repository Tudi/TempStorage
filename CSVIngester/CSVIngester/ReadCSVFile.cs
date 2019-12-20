using CsvHelper;
using System;
using System.Collections.Generic;
using System.Data.SQLite;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

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
        public static string PadASINTo10Chars(string ASIN)
        {
            string ret = "";
            int CharsToAdd = 10 - ASIN.Length;
            for (int i = 0; i < CharsToAdd; i++)
                ret = ret + '0';
            return ret + ASIN;
        }
        public static string FormatDate(string DateCol)
        {
            string ret = DateCol;
            ret = ret.Replace('/', '-');
            ret = ret.Replace(',', '-');
            ret = ret.Replace('.', '-');
            if (ret.Length == 8)
            {
                string[] parts = ret.Split('-');
                if (parts.Length == 3)
                {
                    int year;
                    int.TryParse(parts[2], out year);
                    if (year < 16)
                        GlobalVariables.Logger.Log("Expected year is to be larger than 2016. Found " + year);
                    if (year < 1000)
                    {
                        year += 2000;
                        ret = parts[0] + "-" + parts[1] + "-" + year.ToString();
                    }
                }
            }
            try
            {
                return DateTime.ParseExact(ret, "d-M-yyyy", CultureInfo.InvariantCulture).ToString("dd-MM-yy");
            }
            catch { }
            try
            {
                return DateTime.ParseExact(ret, "dd-MM-yyyy", CultureInfo.InvariantCulture).ToString("dd-MM-yy");
            }
            catch { }
            try
            {
                return DateTime.ParseExact(ret, "dd-MMM-yyyy", CultureInfo.InvariantCulture).ToString("dd-MM-yy");
            }
            catch { }
            try
            {
                return DateTime.Parse(ret).ToString("dd-MM-yy");
            }
            catch { }

            GlobalVariables.Logger.Log("Date format not recognized : " + ret);

            return ret;
        }

        public static void ReadInvenotryCSVFile(string FileName)
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table "+ GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            GlobalVariables.Logger.Log("File import destination database is 'inventory'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string EbayIdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ebay_id");
                if (EbayIdColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'ebay_id' not detected. Not an Inventory csv file : " + FileName);
                    return;
                }
                string AsinColName = GetMatchingColumnName(csv.Context.HeaderRecord, "asin");
                if (AsinColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'asin' not detected.Not an Inventory csv file : " + FileName);
                    return;
                }
                int RowsRead = 0;
                int RowsInserted = 0;
                int RowsUpdated = 0;
                int RowsSkipped = 0;
//                WriteCSVFile ImportResultCSV = new WriteCSVFile();
//                ImportResultCSV.CreateInventoryRunFile("./reports/INVENTORY-RUN.csv");
                GlobalVariables.ImportingToDBBlock = "Inventory";
                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();
                while (csv.Read())
                {
                    string Ebay_id = csv.GetField<string>(EbayIdColName);
                    string asin = csv.GetField<string>(AsinColName);

                    //check if the read data is correct
                    if (Ebay_id == null || Ebay_id.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " ebay_id does not have a value");
                    if (asin == null || asin.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " asin does not have a value");

                    //because some tools will eat up leading 0
                    asin = PadASINTo10Chars(asin);

                    //try to add the new row to the DB
                    DBHandler.InvenotryInsertResultCodes ret = GlobalVariables.DBStorage.InsertInventory(Ebay_id, asin, GlobalVariables.NULLValue.ToString());
                    if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew)
                        RowsInserted++;
                    else if (ret == DBHandler.InvenotryInsertResultCodes.RowExistedButWasEmpty)
                        RowsUpdated++;
                    else
                        RowsSkipped++;
                    RowsRead++;
                    //add to the import log
//                    if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew || ret == DBHandler.InvenotryInsertResultCodes.RowExistedButWasEmpty)
//                        ImportResultCSV.InventoryRunFileAddRow(Ebay_id, asin);
//if (RowsRead == 15)    break;
                }
                GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
                GlobalVariables.Logger.Log("CSV file rows inserted : " + RowsInserted);
                GlobalVariables.Logger.Log("CSV file rows updated : " + RowsUpdated);
                GlobalVariables.Logger.Log("CSV file rows skipped : " + RowsSkipped);
                transaction.Commit();
//                ImportResultCSV.Dispose();
                GlobalVariables.ImportingToDBBlock = "";
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }

        public static void ReadEbayVATCSVFile(string FileName)
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            GlobalVariables.Logger.Log("Ebay VAT File import destination database is 'inventory'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvHelper.CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string EbayIdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ebay_id");
                if (EbayIdColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'ebay_id' not detected.Not an VAT csv file : " + FileName);
                    return;
                }
                string VatColName = GetMatchingColumnName(csv.Context.HeaderRecord, "vat_rate");
                if (VatColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'vat_rate' not detected.Not an VAT csv file : " + FileName);
                    return;
                }
                int RowsRead = 0;
                GlobalVariables.ImportingToDBBlock = "EbayVat";
                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();
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
                transaction.Commit();
                GlobalVariables.ImportingToDBBlock = "";
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }

        public static void ReadAsinVATCSVFile(string FileName)
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }
            GlobalVariables.Logger.Log("Asin VAT File import destination database is 'inventory'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvHelper.CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string AsinColName = GetMatchingColumnName(csv.Context.HeaderRecord, "asin");
                if (AsinColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'asin' not detected.Not an VAT csv file : " + FileName);
                    return;
                }
                string VatColName = GetMatchingColumnName(csv.Context.HeaderRecord, "vat_rate");
                if (VatColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'vat_rate' not detected.Not an VAT csv file : " + FileName);
                    return;
                }
                int RowsRead = 0;
                GlobalVariables.ImportingToDBBlock = "AsinVat";
                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();
                while (csv.Read())
                {
                    string asin = csv.GetField<string>(AsinColName);
                    string vat = csv.GetField<string>(VatColName);

                    //check if the read data is correct
                    if (asin == null || asin.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " asin does not have a value");
                    if (vat == null || vat.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " vat does not have a value");

                    //because some tools will eat up leading 0
                    asin = PadASINTo10Chars(asin);

                    GlobalVariables.DBStorage.UpdateInventoryVatAsin(asin, vat);
//if (RowsRead == 15) break;
                    RowsRead++;
                }
                transaction.Commit();
                GlobalVariables.ImportingToDBBlock = "";
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }

        public static void ReadVATCSVFile(string FileName)
        {
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
                    string EbayIdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ebay_id");
                    string AsinColName = GetMatchingColumnName(csv.Context.HeaderRecord, "asin");
                    string VatColName = GetMatchingColumnName(csv.Context.HeaderRecord, "vat_rate");
                    string Msg = "";
                    if (EbayIdColName.Length == 0 && AsinColName.Length == 0)
                        Msg += "ebay_id or asin";
                    else if (EbayIdColName.Length == 0)
                        Msg += "ebay_id";
                    else if (AsinColName.Length == 0)
                        Msg += "asin";
                    if (VatColName.Length == 0)
                    {
                        if (Msg.Length != 0)
                            Msg += " and ";
                        Msg += "asin";
                    }
                    GlobalVariables.Logger.Log("Mandatory column names 'ebay_id'+'vat_rate' or 'asin'+'vat_rate'. Missing " + Msg + ". Not a VAT csv file : " + FileName);
                    return;
                }
            }
        }

        public static void ReadAmazonOrdersCSVFile(string FileName, string TableName, string CSVFileName, bool UpdateInventory)
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            GlobalVariables.Logger.Log("File import destination database is '" + CSVFileName + "'");
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
                string ToColName = GetMatchingColumnName(csv.Context.HeaderRecord, "To");
                string SellerColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Seller");
                string SellerVATColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Vat Number");

                if (TableName.ToLower() == "Amazon_refunds".ToLower())
                {
                    if (PriceColName.Length == 0)
                        PriceColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Refunded Amount");
                    if (VATColName.Length == 0)
                        VATColName = GetMatchingColumnName(csv.Context.HeaderRecord, "VAT Amount");
                    if (PriceColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Refunded Amount' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                        return;
                    }
                    if (VATColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'VAT Amount' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                        return;
                    }
                }

                if (DateColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Date' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }
                if (IdColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Id' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }
                if (TitleColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Title' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }
                if (PriceColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Price' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }
                if (VATColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'VAT' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }
                if (BuyerColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Buyer' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }
                if (AddressColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Address' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }
                if (ASINColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'ASIN' not detected.Not an " + CSVFileName + " csv file : " + FileName);
                    return;
                }

                GlobalVariables.ImportingToDBBlock = TableName;

//                WriteCSVFile ImportResultCSV = new WriteCSVFile();
//                ImportResultCSV.CreateAmazonOrdersFile("./reports/" + CSVFileName + "-RUN.csv");

                int RowsRead = 0;
                int RowsInserted = 0;
                int RowsSkipped = 0;
                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();
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

                    string SACCOUNT = "";
                    string SELLER = "";
                    string SELLER_VAT = "";
                    if(ToColName != null && ToColName.Length != 0)
                        SACCOUNT = csv.GetField<string>(ToColName);
                    if (SellerColName != null && SellerColName.Length != 0)
                        SELLER = csv.GetField<string>(SellerColName);
                    if (SellerVATColName != null && SellerVATColName.Length != 0)
                        SELLER_VAT = csv.GetField<string>(SellerVATColName);

                    //empty row. Why ?
                    if(DateCol.Length == 0 && IdCol.Length == 0 && TitleCol.Length == 0 && PriceCol.Length == 0 && VATCol.Length == 0 && BuyerCol.Length == 0 && AddressCol.Length == 0 && ASINCol.Length == 0 )
                    {
                        if (SACCOUNT.Length == 0 && SELLER.Length == 0 && SELLER_VAT.Length == 0)
                            continue;
                    }
                    //check if the read data is correct
                    if (DateCol == null || DateCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Date does not have a value");
                    if (IdCol == null || IdCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Id does not have a value");
                    if (TitleCol == null || TitleCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Title does not have a value");
                    if (PriceCol == null || PriceCol.Length == 0)
                    { 
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Price does not have a value");
                        PriceCol = "0";
                    }
                    if (VATCol == null || VATCol.Length == 0)
                    {
                        GlobalVariables.Logger.Log("at line " + RowsRead + " VAT does not have a value");
                        VATCol = "0";
                    }
                    if (BuyerCol == null || BuyerCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Buyer does not have a value");
                    if (AddressCol == null || AddressCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Address does not have a value");
                    if (ASINCol == null || ASINCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " ASIN does not have a value");

                    DateCol = FormatDate(DateCol);

                    //because some tools will eat up leading 0
                    ASINCol = PadASINTo10Chars(ASINCol);

                    double NET = double.Parse(PriceCol) - double.Parse(VATCol);
                    double VAT_RATE = 0;
                    if(NET != 0)
                        VAT_RATE = (int)(0.5 + double.Parse(VATCol) / NET * 100);
                    //!!!!this is a hardcoded request. Maybe should not use rounding ?
                    if (VAT_RATE == 21)
                        VAT_RATE = 20;
                    //check if inventory has missing values that we can update
                    if (UpdateInventory == true)
                        GlobalVariables.DBStorage.UpdateAllMissingInventoryRows(ASINCol, VAT_RATE.ToString());
                    //try to add the new row to the DB
                    DBHandler.InvenotryInsertResultCodes ret = GlobalVariables.DBStorage.InsertAmazonOrder(TableName, DateCol, IdCol, TitleCol, PriceCol, VATCol, BuyerCol, AddressCol, ASINCol, NET, VAT_RATE, SACCOUNT, SELLER, SELLER_VAT);
                    if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew)
                    {
                        RowsInserted++;
//                        ImportResultCSV.AmazonOrdersExportFileAddRow(DateCol, IdCol, TitleCol, double.Parse(PriceCol), double.Parse(VATCol), BuyerCol, AddressCol, ASINCol, NET, VAT_RATE);
                    }
                    else
                        RowsSkipped++;
                    RowsRead++;
                }
                GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
                GlobalVariables.Logger.Log("CSV file rows inserted : " + RowsInserted);
                GlobalVariables.Logger.Log("CSV file rows skipped : " + RowsSkipped);
//                ImportResultCSV.Dispose();
                transaction.Commit();
                GlobalVariables.ImportingToDBBlock = "";
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }

        public static void ReadPaypalSalesCSVFile(string FileName)
        {
            string TableName = "PAYPAL_SALES";
//            string CSVFileName = "PAYPAL-SALES-RUN";
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            GlobalVariables.Logger.Log("File import destination database is '" + TableName + "' and 'PAYPAL_REFUNDS'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                string DateColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Date");
                string NameColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Name");
                string PriceColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Gross");
                string PPFeeColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Fee");
                string TransactionIDColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Transaction ID");
                string TitleColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Item Title");
                string ItemIdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Item ID");
                string AddressColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Shipping Address");
                string PhoneColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Contact Phone Number");
                //not mandatory ?
                string TypeColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Type");
                string BalanceImpactColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Balance Impact");
                string CurrencyColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Currency");
                string ReferenceIDColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Reference Txn ID");
                if (DateColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Date' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (NameColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Name' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (PriceColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Gross' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (PPFeeColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Fee' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (TransactionIDColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Transaction ID' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (TitleColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Item Title' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (ItemIdColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Item ID' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (AddressColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Shipping Address' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (PhoneColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Contact Phone Number' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (TypeColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Type' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (BalanceImpactColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Balance Impact' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (CurrencyColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Currency' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (ReferenceIDColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Reference Txn ID' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                
                GlobalVariables.ImportingToDBBlock = TableName;

//                WriteCSVFile ImportResultCSV = new WriteCSVFile();
//                ImportResultCSV.CreateAmazonOrdersFile("./reports/" + CSVFileName + "-RUN.csv");

                WriteCSVFile SalesMemoCSV = new WriteCSVFile();
                SalesMemoCSV.CreateDynamicFile("./reports/report-sales-memo.csv");
                WriteCSVFile WithDrawMemoCSV = new WriteCSVFile();
                WithDrawMemoCSV.CreateDynamicFile("./reports/report-withdrawals-memo.csv");
                WriteCSVFile WithDrawCSV = new WriteCSVFile();
                WithDrawCSV.CreateDynamicFile("./reports/report-withdrawals.csv");
                WriteCSVFile HoldsCSV = new WriteCSVFile();
                HoldsCSV.CreateDynamicFile("./reports/report-holds.csv");
                WriteCSVFile SalesCSV = new WriteCSVFile();
                SalesCSV.CreateDynamicFile("./reports/report-paypal-sales.csv");
                WriteCSVFile SalesRefundsCSV = new WriteCSVFile();
                SalesRefundsCSV.CreateDynamicFile("./reports/report-paypal-refunds.csv");
                WriteCSVFile RemainingtransactionCSV = new WriteCSVFile();
                RemainingtransactionCSV.CreateDynamicFile("./reports/remaining-transactions.csv ");

                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();

                int RowsRead = 0;
                int RowsInserted = 0;
                int RowsSkipped = 0;
                int RowsRefundInserted = 0;
                int RowsRefundSkipped = 0;
                while (csv.Read())
                {
                    string DateCol = csv.GetField<string>(DateColName);
                    string NameCol = csv.GetField<string>(NameColName);
                    string PriceCol = csv.GetField<string>(PriceColName);
                    string PPFeeCol = csv.GetField<string>(PPFeeColName);
                    string TransactionIDCol = csv.GetField<string>(TransactionIDColName);
                    string TitleCol = csv.GetField<string>(TitleColName);
                    string ItemIdCol = csv.GetField<string>(ItemIdColName);
                    string AddressCol = csv.GetField<string>(AddressColName);
                    string PhoneCol = csv.GetField<string>(PhoneColName);
                    string TypeCol = csv.GetField<string>(TypeColName);
                    string BalanceImpactCol = csv.GetField<string>(BalanceImpactColName);
                    string CurrencyCol = csv.GetField<string>(CurrencyColName);
                    string ReferenceIDCol = csv.GetField<string>(ReferenceIDColName);

                    //check if the read data is correct
/*                    if (DateCol == null || DateCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Date does not have a value");
                    if (NameCol == null || NameCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Name does not have a value");
                    if (PriceCol == null || PriceCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Price does not have a value");
                    if (PPFeeCol == null || PPFeeCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Fee does not have a value");
                    if (TransactionIDCol == null || TransactionIDCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Transaction ID does not have a value");
                    if (TitleCol == null || TitleCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Title does not have a value");
                    if (ItemIdCol == null || ItemIdCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Item Id does not have a value");
                    if (AddressCol == null || AddressCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Shipping Address does not have a value");
                    if (PhoneCol == null || AddressCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Contact Phone does not have a value");
                    if (TypeCol == null || TypeCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Type does not have a value");
                    if (BalanceImpactCol == null || BalanceImpactCol.Length == 0)
                        GlobalVariables.Logger.Log("at line " + RowsRead + " Balance Impact does not have a value");*/

                    DateCol = FormatDate(DateCol);

                    string []MultiIDs = ItemIdCol.Split(',');
                    var ItemIdColOne = MultiIDs[0];
//                    if(ItemIdColOne.Length != 12)
//                        GlobalVariables.Logger.Log("at line " + RowsRead + " ItemId does not have a value");
//                    foreach (var ItemIdColOne in MultiIDs)
                    {
                        if (TypeCol.ToLower() == "eBay Auction Payment".ToLower() && BalanceImpactCol.ToLower() == "Memo".ToLower())
                            SalesMemoCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());
                        else if (TypeCol.ToLower() == "General Withdrawal".ToLower() && BalanceImpactCol.ToLower() == "Memo".ToLower())
                            WithDrawMemoCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());
                        else if (TypeCol.ToLower() == "General Withdrawal".ToLower() && BalanceImpactCol.ToLower() == "Debit".ToLower())
                            WithDrawCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());
                        else if (NameCol.ToLower() == "PayPal".ToLower() && (TypeCol.ToLower() == "Hold on Available Balance".ToLower() || TypeCol.ToLower() == "Reversal of General Account Hold".ToLower()))
                            HoldsCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());
                        else if (double.Parse(PriceCol) > 0 && TypeCol.ToLower() == "eBay Auction Payment".ToLower() && BalanceImpactCol.ToLower() == "Credit".ToLower())
                        {
                            SalesCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());
                            DBHandler.InvenotryInsertResultCodes ret = GlobalVariables.DBStorage.InsertPaypalRow(DateCol, NameCol, PriceCol, PPFeeCol, TransactionIDCol, TitleCol, ItemIdColOne, AddressCol, PhoneCol);
                            if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew)
                                RowsInserted++;
                            else
                                RowsSkipped++;
                        }
                        else if (double.Parse(PriceCol) < 0 && (TypeCol.ToLower() == "Payment Refund".ToLower() || TypeCol.ToLower() == "Payment Reversal".ToLower()) && BalanceImpactCol.ToLower() == "Debit".ToLower() && CurrencyCol.ToLower() == "GBP".ToLower())
                        {
                            SalesRefundsCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());
                            DBHandler.InvenotryInsertResultCodes ret = GlobalVariables.DBStorage.InsertPaypalRefundRow(DateCol, NameCol, PriceCol, PPFeeCol, TransactionIDCol, TitleCol, ItemIdColOne, ReferenceIDCol);
                            if (ret == DBHandler.InvenotryInsertResultCodes.RowDidNotExistInsertedNew)
                                RowsRefundInserted++;
                            else
                                RowsRefundSkipped++;
                        }
                        else
                            RemainingtransactionCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());

                        RowsRead++;
                    }
                }
                GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
                GlobalVariables.Logger.Log("paypal sales rows inserted : " + RowsInserted);
                GlobalVariables.Logger.Log("paypal sales rows skipped : " + RowsSkipped);
                GlobalVariables.Logger.Log("paypal refund rows inserted : " + RowsRefundInserted);
                GlobalVariables.Logger.Log("paypal refund rows skipped : " + RowsRefundSkipped);
                SalesMemoCSV.Dispose();
                WithDrawMemoCSV.Dispose();
                WithDrawCSV.Dispose();
                HoldsCSV.Dispose();
                SalesCSV.Dispose();
                SalesRefundsCSV.Dispose();
                RemainingtransactionCSV.Dispose();
//                ImportResultCSV.Dispose();
                transaction.Commit();
                GlobalVariables.ImportingToDBBlock = "";
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }
    }
}
