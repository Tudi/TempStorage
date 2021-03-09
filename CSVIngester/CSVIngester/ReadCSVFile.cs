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
                if (ExpectedColumnName.ToLower() == itr2.ToLower().TrimStart().TrimEnd())
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
            if (ASIN.Length == 0)
                return "";
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
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
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
                int ReportErrorOnce = 10;
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
                    if (ReportErrorOnce > 0)
                    {
                        if (Ebay_id == null || Ebay_id.Length == 0)
                            GlobalVariables.Logger.Log("at line " + RowsRead + " ebay_id does not have a value");
                        if (asin == null || asin.Length == 0)
                            GlobalVariables.Logger.Log("at line " + RowsRead + " asin does not have a value");
                        if ((Ebay_id == null || Ebay_id.Length == 0) || (asin == null || asin.Length == 0))
                            ReportErrorOnce--;
                        if (ReportErrorOnce == 0)
                            GlobalVariables.Logger.Log("Additional warnings will not be shown to not freez the application");
                    }

                    //because some tools will eat up leading 0
                    asin = PadASINTo10Chars(asin);

                    //try to add the new row to the DB
                    DBHandler.InventoryInsertResultCodes ret = GlobalVariables.DBStorage.InsertInventory(Ebay_id, asin, GlobalVariables.NULLValue.ToString());
                    if (ret == DBHandler.InventoryInsertResultCodes.RowDidNotExistInsertedNew)
                        RowsInserted++;
                    else if (ret == DBHandler.InventoryInsertResultCodes.RowExistedButWasEmpty)
                        RowsUpdated++;
                    else
                        RowsSkipped++;
                    RowsRead++;
                    //add to the import log
                    //                    if (ret == DBHandler.InventoryInsertResultCodes.RowDidNotExistInsertedNew || ret == DBHandler.InventoryInsertResultCodes.RowExistedButWasEmpty)
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
                int ReportErrorOnce = 10;
                GlobalVariables.ImportingToDBBlock = "EbayVat";
                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();
                while (csv.Read())
                {
                    string Ebay_id = csv.GetField<string>(EbayIdColName);
                    string vat = csv.GetField<string>(VatColName);

                    //check if the read data is correct
                    if (ReportErrorOnce > 0)
                    {
                        if (Ebay_id == null || Ebay_id.Length == 0)
                            GlobalVariables.Logger.Log("at line " + RowsRead + " ebay_id does not have a value");
                        if (vat == null || vat.Length == 0)
                            GlobalVariables.Logger.Log("at line " + RowsRead + " vat does not have a value");
                        if ((Ebay_id == null || Ebay_id.Length == 0) || (vat == null || vat.Length == 0))
                            ReportErrorOnce--;
                        if (ReportErrorOnce == 0)
                            GlobalVariables.Logger.Log("Additional warnings will not be shown to not freez the application");
                    }

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
                int ReportErrorOnce = 10;
                int RowsRead = 0;
                GlobalVariables.ImportingToDBBlock = "AsinVat";
                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();
                while (csv.Read())
                {
                    string asin = csv.GetField<string>(AsinColName);
                    string vat = csv.GetField<string>(VatColName);

                    //check if the read data is correct
                    if (ReportErrorOnce > 0)
                    {
                        if (asin == null || asin.Length == 0)
                            GlobalVariables.Logger.Log("at line " + RowsRead + " asin does not have a value");
                        if (vat == null || vat.Length == 0)
                            GlobalVariables.Logger.Log("at line " + RowsRead + " vat does not have a value");
                        if ((asin == null || asin.Length == 0) || (vat == null || vat.Length == 0))
                            ReportErrorOnce--;
                        if (ReportErrorOnce == 0)
                            GlobalVariables.Logger.Log("Additional warnings will not be shown to not freez the application");
                    }

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

                if (CheckHeaderHasColumns(csv.Context.HeaderRecord, ExpectedColumnames1))
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
            CsvHelper.Configuration.Configuration cfg = new CsvHelper.Configuration.Configuration
            {
                HasHeaderRecord = true,
                MissingFieldFound = null
            };
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader, cfg))
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

                int ReportErrorOnce = 10;
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
                    if (ToColName != null && ToColName.Length != 0)
                        SACCOUNT = csv.GetField<string>(ToColName);
                    if (SellerColName != null && SellerColName.Length != 0)
                        SELLER = csv.GetField<string>(SellerColName);
                    if (SellerVATColName != null && SellerVATColName.Length != 0)
                        SELLER_VAT = csv.GetField<string>(SellerVATColName);

                    //empty row. Why ?
                    if (DateCol.Length == 0 && IdCol.Length == 0 && TitleCol.Length == 0 && PriceCol.Length == 0 && VATCol.Length == 0 && BuyerCol.Length == 0 && AddressCol.Length == 0 && ASINCol.Length == 0)
                    {
                        if (SACCOUNT.Length == 0 && SELLER.Length == 0 && SELLER_VAT.Length == 0)
                            continue;
                    }
                    //check if the read data is correct
                    if (ReportErrorOnce > 0)
                    {
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
                        if ((IdCol == null || IdCol.Length == 0) || (TitleCol == null || TitleCol.Length == 0) || (PriceCol == null || PriceCol.Length == 0) || (VATCol == null || VATCol.Length == 0) || (BuyerCol == null || BuyerCol.Length == 0) || (DateCol == null || DateCol.Length == 0) || (ASINCol == null || ASINCol.Length == 0) || (AddressCol == null || AddressCol.Length == 0))
                            ReportErrorOnce--;
                        if (ReportErrorOnce == 0)
                            GlobalVariables.Logger.Log("Additional warnings will not be shown to not freez the application");
                    }
                    if (PriceCol == null || PriceCol.Length == 0)
                        PriceCol = "0";
                    if (VATCol == null || VATCol.Length == 0)
                        VATCol = "0";
                    try
                    {
                        double.Parse(PriceCol);
                        double.Parse(VATCol);
                    }
                    catch
                    {
                        if (ReportErrorOnce > 0)
                        {
                            GlobalVariables.Logger.Log("Vat " + VATCol + " or price " + PriceCol + " is not numeric. Can not import row");
                            ReportErrorOnce--;
                            if (ReportErrorOnce == 0)
                                GlobalVariables.Logger.Log("Additional warnings will not be shown to not freez the application");
                        }
                        continue;
                    }

                    DateCol = FormatDate(DateCol);

                    //because some tools will eat up leading 0
                    ASINCol = PadASINTo10Chars(ASINCol);

                    double NET = double.Parse(PriceCol) - double.Parse(VATCol);
                    double VAT_RATE = 0;
                    if (NET != 0)
                        VAT_RATE = (int)(0.5 + double.Parse(VATCol) / NET * 100);
                    //!!!!this is a hardcoded request. Maybe should not use rounding ?
                    if (VAT_RATE == 21)
                        VAT_RATE = 20;
                    //check if inventory has missing values that we can update
                    if (UpdateInventory == true)
                        GlobalVariables.DBStorage.UpdateAllMissingInventoryRows(ASINCol, VAT_RATE.ToString());

                    //check if this row is blocked from import
                    DBHandler.InventoryInsertResultCodes RowImportIsBlocked = GlobalVariables.DBStorage.IsAmazonOrderBlocked(IdCol);
                    if (RowImportIsBlocked == DBHandler.InventoryInsertResultCodes.RowDidNotExist)
                    {
                        //try to add the new row to the DB
                        DBHandler.InventoryInsertResultCodes ret = GlobalVariables.DBStorage.InsertAmazonOrder(TableName, DateCol, IdCol, TitleCol, PriceCol, VATCol, BuyerCol, AddressCol, ASINCol, NET, VAT_RATE, SACCOUNT, SELLER, SELLER_VAT);
                        if (ret == DBHandler.InventoryInsertResultCodes.RowDidNotExistInsertedNew)
                        {
                            RowsInserted++;
                            //                        ImportResultCSV.AmazonOrdersExportFileAddRow(DateCol, IdCol, TitleCol, double.Parse(PriceCol), double.Parse(VATCol), BuyerCol, AddressCol, ASINCol, NET, VAT_RATE);
                        }
                        else
                            RowsSkipped++;
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

#if USE_CACHE
            GlobalVariables.CachedIndexes.CacheExistingIndexes(CachedIndexTypes.CIT_PaypalSales);
#endif

            GlobalVariables.Logger.Log("File import destination database is '" + TableName + "' and 'PAYPAL_REFUNDS'");
            string HeaderRow;
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader))
            {
                csv.Read();
                csv.ReadHeader();
                HeaderRow = csv.Context.RawRecord;
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
                SalesMemoCSV.WriteLine(HeaderRow);
                WriteCSVFile WithDrawMemoCSV = new WriteCSVFile();
                WithDrawMemoCSV.CreateDynamicFile("./reports/report-withdrawals-memo.csv");
                WithDrawMemoCSV.WriteLine(HeaderRow);
                WriteCSVFile WithDrawCSV = new WriteCSVFile();
                WithDrawCSV.CreateDynamicFile("./reports/report-withdrawals.csv");
                WithDrawCSV.WriteLine(HeaderRow);
                WriteCSVFile HoldsCSV = new WriteCSVFile();
                HoldsCSV.CreateDynamicFile("./reports/report-holds.csv");
                HoldsCSV.WriteLine(HeaderRow);
                WriteCSVFile SalesCSV = new WriteCSVFile();
                SalesCSV.CreateDynamicFile("./reports/report-paypal-sales.csv");
                SalesCSV.WriteLine(HeaderRow);
                WriteCSVFile SalesRefundsCSV = new WriteCSVFile();
                SalesRefundsCSV.CreateDynamicFile("./reports/report-paypal-refunds.csv");
                SalesRefundsCSV.WriteLine(HeaderRow);
                WriteCSVFile RemainingtransactionCSV = new WriteCSVFile();
                RemainingtransactionCSV.CreateDynamicFile("./reports/remaining-transactions.csv ");
                RemainingtransactionCSV.WriteLine(HeaderRow);

                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();

                int ReportErrorOnce = 10;
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
                    if (ReportErrorOnce > 0)
                    {
                        if (DateCol == null || DateCol.Length == 0)
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
                            GlobalVariables.Logger.Log("at line " + RowsRead + " Balance Impact does not have a value");
                        if ((DateCol == null || DateCol.Length == 0) || (NameCol == null || NameCol.Length == 0) || (PriceCol == null || PriceCol.Length == 0) || (PPFeeCol == null || PPFeeCol.Length == 0) || (TransactionIDCol == null || TransactionIDCol.Length == 0) || (TitleCol == null || TitleCol.Length == 0) || (ItemIdCol == null || ItemIdCol.Length == 0) || (AddressCol == null || AddressCol.Length == 0) || (PhoneCol == null || AddressCol.Length == 0) || (TypeCol == null || TypeCol.Length == 0) || (BalanceImpactCol == null || BalanceImpactCol.Length == 0))
                            ReportErrorOnce--;
                        if (ReportErrorOnce == 0)
                            GlobalVariables.Logger.Log("Additional warnings will not be shown to not freez the application");
                    }

                    DateCol = FormatDate(DateCol);

                    string[] MultiIDs = ItemIdCol.Split(',');
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
                            DBHandler.InventoryInsertResultCodes ret = GlobalVariables.DBStorage.InsertPaypalRow(DateCol, NameCol, PriceCol, PPFeeCol, TransactionIDCol, TitleCol, ItemIdColOne, AddressCol, PhoneCol);
                            if (ret == DBHandler.InventoryInsertResultCodes.RowDidNotExistInsertedNew)
                                RowsInserted++;
                            else
                                RowsSkipped++;
                        }
                        else if (double.Parse(PriceCol) < 0 && (TypeCol.ToLower() == "Payment Refund".ToLower() || TypeCol.ToLower() == "Payment Reversal".ToLower()) && BalanceImpactCol.ToLower() == "Debit".ToLower() && CurrencyCol.ToLower() == "GBP".ToLower())
                        {
                            SalesRefundsCSV.WriteDynamicFileRow(csv.GetRecord<dynamic>());
                            DBHandler.InventoryInsertResultCodes ret = GlobalVariables.DBStorage.InsertPaypalRefundRow(DateCol, NameCol, PriceCol, PPFeeCol, TransactionIDCol, TitleCol, ItemIdColOne, ReferenceIDCol);
                            if (ret == DBHandler.InventoryInsertResultCodes.RowDidNotExistInsertedNew)
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

        public static void ReadAmazonDeleteCSVFile(string FileName)
        {
            string TableName = "AMAZON_BLOCKED";
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            CsvHelper.Configuration.Configuration cfg = new CsvHelper.Configuration.Configuration
            {
                HasHeaderRecord = true,
                MissingFieldFound = null
            };

            GlobalVariables.Logger.Log("File import destination database is '" + TableName);
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader, cfg))
            {
                csv.Read();
                csv.ReadHeader();
                string IdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Id");
                string DispatchedColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Dispatched");
                string DateColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Date");
                string TitleColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Title");
                string PriceColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Price");
                string VatColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Vat Amount");
                string BuyerColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Buyer");
                string AddressColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Address");
                string ASINColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ASIN");
                string PaymentColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Payment Method");
                if (IdColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Id' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (DispatchedColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'Dispatched' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }

                GlobalVariables.ImportingToDBBlock = TableName;

                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();

                int RowsRead = 0;
                int RowsReadValid = 0;
                int RowsInserted = 0;
                int RowsSkipped = 0;
                while (csv.Read())
                {
                    string IdCol = csv.GetField<string>(IdColName);
                    string DispatchedCol = csv.GetField<string>(DispatchedColName);
                    string DateCol = csv.GetField<string>(DateColName);
                    string TitleCol = csv.GetField<string>(TitleColName);
                    string PriceCol = csv.GetField<string>(PriceColName);
                    string VatCol = csv.GetField<string>(VatColName);
                    string BuyerCol = csv.GetField<string>(BuyerColName);
                    string AddressCol = csv.GetField<string>(AddressColName);
                    string ASINCol = csv.GetField<string>(ASINColName);
                    string PaymentCol = csv.GetField<string>(PaymentColName);

                    if (IdCol == null || IdCol.Length == 0)
                        continue;
                    if (DispatchedCol == null || DispatchedCol.Length == 0)
                        continue;
                    //checking expected values
                    if (DispatchedCol.ToLower() != "True".ToLower() && DispatchedCol.ToLower() != "False".ToLower())
                        continue;
                    RowsRead++;

                    //only process false rows ( i know we just checked, For the sake of code readability .. )
                    if (DispatchedCol.ToLower() != "False".ToLower())
                    {
                        //                        DBHandler.InventoryInsertResultCodes ret2 = GlobalVariables.DBStorage.IsAmazonOrderBlocked(IdCol);
                        //                        if(ret2 == DBHandler.InventoryInsertResultCodes.RowExisted)
                        GlobalVariables.DBStorage.DeleteAmazonOrderBlocked(IdCol);
                        continue;
                    }

                    //try to add the new row to the DB
                    DBHandler.InventoryInsertResultCodes ret1 = GlobalVariables.DBStorage.InsertAmazonBlocked("AMAZON_BLOCKED", IdCol, DispatchedCol, DateCol, TitleCol, PriceCol, VatCol, BuyerCol, AddressCol, ASINCol, PaymentCol);

                    //delete the order if already exists
                    DBHandler.InventoryInsertResultCodes ret = GlobalVariables.DBStorage.DeleteAmazonOrder(IdCol);
                    if (ret == DBHandler.InventoryInsertResultCodes.RowDeleted)
                        RowsInserted++;
                    else
                        RowsSkipped++;

                    RowsReadValid++;
                }
                GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
                GlobalVariables.Logger.Log("CSV file rows FALSE : " + RowsReadValid);
                GlobalVariables.Logger.Log("CSV file rows FALSE deleted : " + RowsInserted);
                GlobalVariables.Logger.Log("CSV file rows FALSE skipped : " + RowsSkipped);
                transaction.Commit();
                GlobalVariables.ImportingToDBBlock = "";
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }
        public static void ReadIManagedCSVFile(string FileName)
        {
            string TableName = "INVENTORY";
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            CsvHelper.Configuration.Configuration cfg = new CsvHelper.Configuration.Configuration
            {
                HasHeaderRecord = true,
                MissingFieldFound = null
            };

            GlobalVariables.Logger.Log("File import destination database is '" + TableName + "'");
            using (var reader = new StreamReader(FileName))
            using (var csv = new CsvReader(reader, cfg))
            {
                csv.Read();
                csv.ReadHeader();
                string EbayColName = GetMatchingColumnName(csv.Context.HeaderRecord, "ebay id");
                string SourceColName = GetMatchingColumnName(csv.Context.HeaderRecord, "source id");
                if (EbayColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'ebay id' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }
                if (SourceColName.Length == 0)
                {
                    GlobalVariables.Logger.Log("Mandatory column 'source id' not detected.Not an " + TableName + " csv file : " + FileName);
                    return;
                }

                GlobalVariables.ImportingToDBBlock = TableName;

                SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();

                int RowsRead = 0;
                int RowsReadValid = 0;
                int RowsInserted = 0;
                int RowsSkipped = 0;
                int RowsUpdated = 0;
                WriteCSVFile ImportResultCSV = new WriteCSVFile();
                ImportResultCSV.CreateDynamicFile("./reports/IMANAGED-RUN.csv");
                ImportResultCSV.WriteLine("ebay_id,asin,vat\n");
                while (csv.Read())
                {
                    string EbayCol = csv.GetField<string>(EbayColName);
                    string SourceCol = csv.GetField<string>(SourceColName);

                    RowsRead++;

                    if (EbayCol == null || EbayCol.Length == 0)
                        continue;
                    if (SourceCol == null || SourceCol.Length == 0)
                        continue;

                    RowsReadValid++;

                    // Import row
                    DBHandler.InventoryInsertResultCodes ret = GlobalVariables.DBStorage.InsertInventory(EbayCol, SourceCol, GlobalVariables.NULLValue.ToString());
                    if (ret == DBHandler.InventoryInsertResultCodes.RowDidNotExistInsertedNew)
                    {
                        RowsInserted++;
                        ImportResultCSV.WriteLine(EbayCol+","+ SourceCol+",\n");
                    }
                    else if (ret == DBHandler.InventoryInsertResultCodes.RowExistedButWasEmpty)
                        RowsUpdated++;
                    else
                        RowsSkipped++;
                }
                GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
                GlobalVariables.Logger.Log("CSV file rows valid: " + RowsReadValid);
                GlobalVariables.Logger.Log("CSV file rows inserted : " + RowsInserted);
                GlobalVariables.Logger.Log("CSV file rows skipped : " + RowsSkipped);
                transaction.Commit();
                ImportResultCSV.Dispose();
                GlobalVariables.ImportingToDBBlock = "";
            }
            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }
        enum EbayColumnsUsed
        {
            Type = 0,
            ItemSubTotal,
            Postage,
            FinalValueFixed,
            FinalValueVariable,
            ItemNotAsDescribedFee,
            ItemBelowPerformanceFee,
            ItemInternationalFee,
            ReasonForHold,
            Date,
            Name,
            Address,
            TransactionId,
            ItemId,
            ValGross,
            WholeRow
        }

        public static double DoubleParseEbay(string s)
        {
            if (s.Length == 0)
                return 0;
            else if (s == "--")
                return 0;
            return double.Parse(s);
        }

        public static void ReadEbayCSVFile(string FileName)
        {
            string TableName = "EBAY_SALES";
            //            string CSVFileName = "PAYPAL-SALES-RUN";
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }
    
            GlobalVariables.Logger.Log("File import destination database is '" + TableName + "' and 'EBAY_REFUNDS'");

            //  As first step, we will read it into memory
            var CsvList = new List<Dictionary<EbayColumnsUsed, string>>();
            string HeaderRow = "";
            using (var reader = new StreamReader(FileName))
            {
                using (var csv = new CsvReader(reader))
                {
                    // Is the first line the header or is it the version where it's 10 lines worth of comments ?
                    csv.Read();
                    csv.ReadHeader();
                    string DateColNameCheck = GetMatchingColumnName(csv.Context.HeaderRecord, "Transaction date");
                    if (DateColNameCheck.Length == 0)
                    {
                        // Skip first 10 lines
                        for (int i = 0; i < 9; i++)
                            csv.Read();

                        csv.Read();
                        csv.ReadHeader();
                    }

                    HeaderRow = csv.Context.RawRecord;
/*                    string DateColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Transaction date");
                    string NameColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Buyer username");
                    string AddressColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Post to postcode");
                    string TransactionIDColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Order number");
                    string ItemIdColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Item ID");
                    string PriceColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Gross transaction amount");
                    string FVFeeFixedColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Final value fee – fixed");
                    string FVFeeVariableColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Final value fee – variable");
                    string FeeItemNotAsDescribedColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Very high 'item not as described' fee");
                    string FeeBelowPerformanceColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Below standard performance fee");
                    string FeeInternationalColName = GetMatchingColumnName(csv.Context.HeaderRecord, "International fee");
                    string TypeColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Type");
                    string ItemSubtotalColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Item subtotal");
                    string PostageColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Postage and packaging");
                    string ReasonForHoldColName = GetMatchingColumnName(csv.Context.HeaderRecord, "Reason for hold");

                    // check if all manadatory columns have values
                    if (DateColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Transaction date' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (NameColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Buyer username' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (AddressColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Post to postcode' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (TransactionIDColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Order number' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (ItemIdColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Item ID' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (PriceColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Gross transaction amount' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (FVFeeFixedColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Final value fee – fixed' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (FVFeeVariableColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Final value fee – variable' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (FeeBelowPerformanceColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'Below standard performance fee' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (FeeInternationalColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'International fee' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    }
                    if (FeeInternationalColName.Length == 0)
                    {
                        GlobalVariables.Logger.Log("Mandatory column 'International fee' not detected.Not an " + TableName + " csv file : " + FileName);
                        return;
                    } */

                    while (csv.Read())
                    {
                        Dictionary<EbayColumnsUsed, string> CSVRow = new Dictionary<EbayColumnsUsed, string>();

                        /*                        CSVRow[EbayColumnsUsed.Date] = csv.GetField<string>(DateColName);
                                                CSVRow[EbayColumnsUsed.Name] = csv.GetField<string>(NameColName);
                                                CSVRow[EbayColumnsUsed.Address] = csv.GetField<string>(AddressColName);
                                                CSVRow[EbayColumnsUsed.TransactionId] = csv.GetField<string>(TransactionIDColName);
                                                CSVRow[EbayColumnsUsed.ItemId] = csv.GetField<string>(ItemIdColName);
                                                CSVRow[EbayColumnsUsed.ValGross] = csv.GetField<string>(PriceColName);
                                                CSVRow[EbayColumnsUsed.FinalValueFixed] = csv.GetField<string>(FVFeeFixedColName);
                                                CSVRow[EbayColumnsUsed.FinalValueVariable] = csv.GetField<string>(FVFeeVariableColName);
                                                CSVRow[EbayColumnsUsed.ItemNotAsDescribedFee] = csv.GetField<string>(FeeItemNotAsDescribedColName);
                                                CSVRow[EbayColumnsUsed.ItemBelowPerformanceFee] = csv.GetField<string>(FeeBelowPerformanceColName);
                                                CSVRow[EbayColumnsUsed.ItemInternationalFee] = csv.GetField<string>(FeeInternationalColName);
                                                CSVRow[EbayColumnsUsed.Type] = csv.GetField<string>(TypeColName);
                                                CSVRow[EbayColumnsUsed.ItemSubTotal] = csv.GetField<string>(ItemSubtotalColName);
                                                CSVRow[EbayColumnsUsed.Postage] = csv.GetField<string>(PostageColName);
                                                CSVRow[EbayColumnsUsed.ReasonForHold] = csv.GetField<string>(ReasonForHoldColName);*/

                        CSVRow[EbayColumnsUsed.Date] = csv.GetField<string>('a'-'a');
                        CSVRow[EbayColumnsUsed.Name] = csv.GetField<string>('e'-'a');
                        CSVRow[EbayColumnsUsed.Address] = csv.GetField<string>('i'-'a');
                        CSVRow[EbayColumnsUsed.TransactionId] = csv.GetField<string>('c'-'a');
                        CSVRow[EbayColumnsUsed.ItemId] = csv.GetField<string>('r'-'a');
                        CSVRow[EbayColumnsUsed.ValGross] = csv.GetField<string>('z'-'a'+'g'-'a'+1);
                        CSVRow[EbayColumnsUsed.FinalValueFixed] = csv.GetField<string>('z' - 'a' + 'b' - 'a' + 1);
                        CSVRow[EbayColumnsUsed.FinalValueVariable] = csv.GetField<string>('z' - 'a' + 'c' - 'a' + 1);
                        CSVRow[EbayColumnsUsed.ItemNotAsDescribedFee] = csv.GetField<string>('z' - 'a' + 'd' - 'a' + 1);
                        CSVRow[EbayColumnsUsed.ItemBelowPerformanceFee] = csv.GetField<string>('z' - 'a' + 'e' - 'a' + 1);
                        CSVRow[EbayColumnsUsed.ItemInternationalFee] = csv.GetField<string>('z' - 'a' + 'f' - 'a' + 1);
                        CSVRow[EbayColumnsUsed.Type] = csv.GetField<string>('b' - 'a');
                        CSVRow[EbayColumnsUsed.ItemSubTotal] = csv.GetField<string>('w' - 'a');
                        CSVRow[EbayColumnsUsed.Postage] = csv.GetField<string>('x' - 'a');
                        CSVRow[EbayColumnsUsed.ReasonForHold] = csv.GetField<string>('q' - 'a');

                        CSVRow[EbayColumnsUsed.WholeRow] = csv.Context.RawRecord;

                        // Syntax from ebay to mark empty fields is --
                        bool MadeChange = false;
                        do
                        {
                            MadeChange = false;
                            foreach (KeyValuePair<EbayColumnsUsed, string> entry in CSVRow)
                                if (entry.Value == "--")
                                {
                                    CSVRow[entry.Key] = "";
                                    MadeChange = true;
                                    break;
                                }
                        } while (MadeChange == true);

                        CsvList.Add(CSVRow);
                    }
                }
            }

            // Try to fix missing values                   
            List<Dictionary<EbayColumnsUsed, string>>.Enumerator itr1 = CsvList.GetEnumerator();
            while (itr1.MoveNext() == true)
            {
                Dictionary<EbayColumnsUsed, string> row = itr1.Current;
                double ItemSubtotal = DoubleParseEbay(row[EbayColumnsUsed.ItemSubTotal]);
                if (ItemSubtotal == 0 && row[EbayColumnsUsed.Type] == "Order")
                {
                    List<Dictionary<EbayColumnsUsed, string>>.Enumerator itr2 = itr1;
                    string MissingItemId = "";
                    double SumSubtotal = 0;
                    double SumPostage = 0;
                    double SumFinalValueFeeFixed = 0;
                    double SumFinalValueFeeVariable = 0;
                    double SumItemNotAsDescribedFee = 0;
                    double SumItemBelowPerformanceFee = 0;
                    double SumItemInternationalFee = 0;
                    while (itr2.MoveNext() == true)
                    {
                        Dictionary<EbayColumnsUsed, string> row2 = itr2.Current;
                        if (row2[EbayColumnsUsed.Type].Length == 0 && row2[EbayColumnsUsed.TransactionId] == row[EbayColumnsUsed.TransactionId])
                        {
                            if (MissingItemId.Length == 0 && row2[EbayColumnsUsed.ItemId].Length != 0)
                            {
                                MissingItemId = row2[EbayColumnsUsed.ItemId];
                            }
                            SumSubtotal += DoubleParseEbay(row2[EbayColumnsUsed.ItemSubTotal]);
                            SumPostage += DoubleParseEbay(row2[EbayColumnsUsed.Postage]);
                            SumFinalValueFeeFixed += DoubleParseEbay(row2[EbayColumnsUsed.FinalValueFixed]);
                            SumFinalValueFeeVariable += DoubleParseEbay(row2[EbayColumnsUsed.FinalValueVariable]);
                            SumItemNotAsDescribedFee += DoubleParseEbay(row2[EbayColumnsUsed.ItemNotAsDescribedFee]);
                            SumItemBelowPerformanceFee += DoubleParseEbay(row2[EbayColumnsUsed.ItemBelowPerformanceFee]);
                            SumItemInternationalFee += DoubleParseEbay(row2[EbayColumnsUsed.ItemInternationalFee]);
                        }
                        else
                        {
                            break;

                        }
                    } 
                    itr1.Current[EbayColumnsUsed.ItemId] = MissingItemId.ToString();
                    itr1.Current[EbayColumnsUsed.ItemSubTotal] = SumSubtotal.ToString();
                    itr1.Current[EbayColumnsUsed.Postage] = SumPostage.ToString();
                    itr1.Current[EbayColumnsUsed.FinalValueFixed] = SumFinalValueFeeFixed.ToString();
                    itr1.Current[EbayColumnsUsed.FinalValueVariable] = SumFinalValueFeeVariable.ToString();
                    itr1.Current[EbayColumnsUsed.ItemNotAsDescribedFee] = SumItemNotAsDescribedFee.ToString();
                    itr1.Current[EbayColumnsUsed.ItemBelowPerformanceFee] = SumItemBelowPerformanceFee.ToString();
                    itr1.Current[EbayColumnsUsed.ItemInternationalFee] = SumItemInternationalFee.ToString();
                }

            }

            // Process the fixed values as they are from the file

            int RowsRead = 0;
            int RowsInserted = 0;
            int RowsUpdated = 0;
            int RowsSkipped = 0;
            int SalesImported = 0;
            int SalesSkipped = 0;
            int RefundsImported = 0;
            int RefundsSkipped = 0;
            int ClaimsImported = 0;
            int ClaimsSkipped = 0;
            int DisputesImported = 0;
            int DisputesSkipped = 0;

            WriteCSVFile HoldsCSV = new WriteCSVFile();
            HoldsCSV.CreateDynamicFile("./reports/report-ebay-hold.csv");
            HoldsCSV.WriteLine(HeaderRow);
            WriteCSVFile SalesHoldsCSV = new WriteCSVFile();
            SalesHoldsCSV.CreateDynamicFile("./reports/report-ebay-sales-hold.csv");
            SalesHoldsCSV.WriteLine(HeaderRow);
//            WriteCSVFile SalesCSV = new WriteCSVFile();
//            SalesCSV.CreateDynamicFile("./reports/report-ebay-sales.csv");
//            SalesCSV.WriteLine(HeaderRow);
            WriteCSVFile SalesRunCSV = new WriteCSVFile();
            SalesRunCSV.CreateDynamicFile("./reports/report-ebay-sales-run.csv");
            SalesRunCSV.WriteLine(HeaderRow);
//            WriteCSVFile RefundCSV = new WriteCSVFile();
//            RefundCSV.CreateDynamicFile("./reports/report-ebay-refunds.csv");
//            RefundCSV.WriteLine(HeaderRow);
            WriteCSVFile RefundRunCSV = new WriteCSVFile();
            RefundRunCSV.CreateDynamicFile("./reports/report-ebay-refunds-run.csv");
            RefundRunCSV.WriteLine(HeaderRow);
//            WriteCSVFile ClaimCSV = new WriteCSVFile();
//            ClaimCSV.CreateDynamicFile("./reports/report-ebay-claim.csv");
//            ClaimCSV.WriteLine(HeaderRow);
            WriteCSVFile ClaimRunCSV = new WriteCSVFile();
            ClaimRunCSV.CreateDynamicFile("./reports/report-ebay-claim-run.csv");
            ClaimRunCSV.WriteLine(HeaderRow);
//            WriteCSVFile DisputesCSV = new WriteCSVFile();
//            DisputesCSV.CreateDynamicFile("./reports/report-ebay-disputes.csv");
//            DisputesCSV.WriteLine(HeaderRow);
            WriteCSVFile DisputesRunCSV = new WriteCSVFile();
            DisputesRunCSV.CreateDynamicFile("./reports/report-ebay-disputes-run.csv");
            DisputesRunCSV.WriteLine(HeaderRow);
            WriteCSVFile UnknownCSV = new WriteCSVFile();
            UnknownCSV.CreateDynamicFile("./reports/report-ebay-remaining-transactions.csv");
            UnknownCSV.WriteLine(HeaderRow);

            GlobalVariables.ImportingToDBBlock = TableName;
            SQLiteTransaction transaction = GlobalVariables.DBStorage.m_dbConnection.BeginTransaction();

            // Rows are ordered older to newer. Refunds appear before "orders"
            CsvList.Reverse();

            foreach (var row in CsvList)
            {
                RowsRead++;

                if (row[EbayColumnsUsed.Type] == "Hold")
                {
                    HoldsCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                    RowsSkipped++;
                    continue;
                }

                double FeeGross = DoubleParseEbay(row[EbayColumnsUsed.FinalValueFixed]);
                FeeGross += DoubleParseEbay(row[EbayColumnsUsed.FinalValueVariable]);
                FeeGross += DoubleParseEbay(row[EbayColumnsUsed.ItemNotAsDescribedFee]);
                FeeGross += DoubleParseEbay(row[EbayColumnsUsed.ItemBelowPerformanceFee]);
                FeeGross += DoubleParseEbay(row[EbayColumnsUsed.ItemInternationalFee]);

//                string InsertIntoTableName = "";
                string ItemId = row[EbayColumnsUsed.ItemId];
                if (row[EbayColumnsUsed.Type] == "Order")
                {
                    if (row[EbayColumnsUsed.ReasonForHold].Length != 0)
                    {
                        SalesHoldsCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                        RowsSkipped++;
                        continue;
                    }
                    else if (row[EbayColumnsUsed.ReasonForHold].Length == 0)
                    {
                        // For manual checking 
//                        SalesCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);

                        string AlreadyInserted = GlobalVariables.DBStorage.EbayGetItemID(row[EbayColumnsUsed.TransactionId]);
                        if (AlreadyInserted.Length > 0)
                        {
                            RowsSkipped++;
                            SalesSkipped++;
                            continue;
                        }

                        SalesImported++;
                        SalesRunCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                        GlobalVariables.DBStorage.ReplaceEbaySale(
                            row[EbayColumnsUsed.Date],
                            row[EbayColumnsUsed.Name],
                            row[EbayColumnsUsed.Address],
                            row[EbayColumnsUsed.TransactionId],
                            ItemId,
                            row[EbayColumnsUsed.ValGross],
                            FeeGross,
                            GlobalVariables.NULLValue,
                            GlobalVariables.NULLValue,
                            GlobalVariables.NULLValue
                            );
                        RowsInserted++;
                    }
                }


                else if (row[EbayColumnsUsed.Type] == "Refund")
                {
//                    RefundCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                    bool AlreadyInserted = GlobalVariables.DBStorage.EbayCheckRefundExists(row[EbayColumnsUsed.Date], row[EbayColumnsUsed.Type], row[EbayColumnsUsed.TransactionId], row[EbayColumnsUsed.ValGross]);
                    if (AlreadyInserted == true)
                    {
                        RowsSkipped++;
                        RefundsSkipped++;
                        continue;
                    }

                    RefundsImported++;
                    // Get item id from sales for this refund
                    ItemId = GlobalVariables.DBStorage.EbayGetItemID(row[EbayColumnsUsed.TransactionId]);
                    
                    RefundRunCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                    
                    GlobalVariables.DBStorage.ReplaceEbayRefund(row[EbayColumnsUsed.Type],
                        row[EbayColumnsUsed.Date],
                        row[EbayColumnsUsed.Name],
                        row[EbayColumnsUsed.Address],
                        row[EbayColumnsUsed.TransactionId],
                        ItemId,
                        row[EbayColumnsUsed.ValGross],
                        FeeGross,
                        GlobalVariables.NULLValue,
                        GlobalVariables.NULLValue,
                        GlobalVariables.NULLValue
                        );
                    
                    RowsInserted++;
                }


                else if (row[EbayColumnsUsed.Type] == "Claim")
                {
//                    ClaimCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                    // check if this claim is already present in the db
                    bool AlreadyInserted = GlobalVariables.DBStorage.EbayCheckRefundExists(row[EbayColumnsUsed.Date], row[EbayColumnsUsed.Type], row[EbayColumnsUsed.TransactionId], row[EbayColumnsUsed.ValGross]);
                    if (AlreadyInserted == true)
                    {
                        RowsSkipped++;
                        ClaimsSkipped++;
                        continue;
                    }
                    ClaimsImported++;

                    // Get item id from sales for this refund
                    ItemId = GlobalVariables.DBStorage.EbayGetItemID(row[EbayColumnsUsed.TransactionId]);

                    // save to file what we insert
                    ClaimRunCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);

                    GlobalVariables.DBStorage.ReplaceEbayRefund(row[EbayColumnsUsed.Type],
                        row[EbayColumnsUsed.Date],
                        row[EbayColumnsUsed.Name],
                        row[EbayColumnsUsed.Address],
                        row[EbayColumnsUsed.TransactionId],
                        ItemId,
                        row[EbayColumnsUsed.ValGross],
                        FeeGross,
                        GlobalVariables.NULLValue,
                        GlobalVariables.NULLValue,
                        GlobalVariables.NULLValue
                        );
                    RowsInserted++;
                }


                else if (row[EbayColumnsUsed.Type] == "Payment dispute")
                {
//                    DisputesCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                    bool AlreadyInserted = GlobalVariables.DBStorage.EbayCheckRefundExists(row[EbayColumnsUsed.Date], row[EbayColumnsUsed.Type], row[EbayColumnsUsed.TransactionId], row[EbayColumnsUsed.ValGross]);
                    if (AlreadyInserted == true)
                    {
                        RowsSkipped++;
                        DisputesSkipped++;
                        continue;
                    }

                    DisputesImported++;

                    // Get item id from sales for this refund
                    ItemId = GlobalVariables.DBStorage.EbayGetItemID(row[EbayColumnsUsed.TransactionId]);

                    DisputesRunCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);

                    GlobalVariables.DBStorage.ReplaceEbayRefund(row[EbayColumnsUsed.Type],
                        row[EbayColumnsUsed.Date],
                        row[EbayColumnsUsed.Name],
                        row[EbayColumnsUsed.Address],
                        row[EbayColumnsUsed.TransactionId],
                        ItemId,
                        row[EbayColumnsUsed.ValGross],
                        FeeGross,
                        GlobalVariables.NULLValue,
                        GlobalVariables.NULLValue,
                        GlobalVariables.NULLValue
                        );

                    RowsInserted++;
                }
                else
                {
                    UnknownCSV.WriteLine(row[EbayColumnsUsed.WholeRow]);
                    RowsSkipped++;
                    continue;
                }
            }
            transaction.Commit();
            GlobalVariables.ImportingToDBBlock = "";

            GlobalVariables.Logger.Log("CSV file rows : " + RowsRead);
            GlobalVariables.Logger.Log("CSV file rows inserted : " + RowsInserted);
            GlobalVariables.Logger.Log("CSV file rows updated : " + RowsUpdated);
            GlobalVariables.Logger.Log("CSV file rows skipped : " + RowsSkipped);
            GlobalVariables.Logger.Log("Sales imported : " + SalesImported);
            GlobalVariables.Logger.Log("Sales skipped : " + SalesSkipped);
            GlobalVariables.Logger.Log("Refunds imported : " + RefundsImported);
            GlobalVariables.Logger.Log("Refunds skipped : " + RefundsSkipped);
            GlobalVariables.Logger.Log("Claims imported : " + ClaimsImported);
            GlobalVariables.Logger.Log("Claims skipped : " + ClaimsSkipped);
            GlobalVariables.Logger.Log("Disputes imported : " + DisputesImported);
            GlobalVariables.Logger.Log("Disputes skipped : " + DisputesSkipped);

            HoldsCSV.Dispose();
            SalesHoldsCSV.Dispose();
//            SalesCSV.Dispose();
//            RefundCSV.Dispose();
            RefundRunCSV.Dispose();
            SalesRunCSV.Dispose();
            ClaimRunCSV.Dispose();
//            ClaimCSV.Dispose();
//            DisputesCSV.Dispose();
            DisputesRunCSV.Dispose();
            UnknownCSV.Dispose();

            GlobalVariables.Logger.Log("Finished importing file : " + FileName);
        }
    }
}

