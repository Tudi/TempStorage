using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SQLite;
using System.Transactions;
using System.Globalization;
using System.Diagnostics;

namespace CSVIngester
{
    public static class DateParser
    {
        public static long DateTimeToMinutes(DateTime dt)
        {
            return (long)(dt - new DateTime(1970, 1, 1, 0, 0, 0)).TotalMinutes;
        }
        public static long GetUnixStamp(string DateStr)
        {
            long ret = DateTimeToMinutes(new DateTime(2001, 1, 2));
            long retInitial = ret;
            DateStr = DateStr.Replace('/', '-');
            DateStr = DateStr.Replace('\\', '-');
            DateStr = DateStr.Replace("\n", "");
            //expected date is dd/MM/YY
            if (DateStr.Length == 8)
            {
                string[] parts = DateStr.Split('-');
                if (parts.Length == 3)
                {
                    //maybe day/month/year
                    int day;
                    int month;
                    int year;
                    {
                        int.TryParse(parts[0], out day);
                        int.TryParse(parts[1], out month);
                        int.TryParse(parts[2], out year);
                        year += 2000;
                        ret = DateTimeToMinutes(new DateTime(year, month, day));
                    }
                }
            }
            if (ret == retInitial)
            {
                try
                {
                    DateTime dt = DateTime.ParseExact(DateStr, "dd-MM-yyyy", CultureInfo.InvariantCulture);
                    ret = DateTimeToMinutes(dt);
                }
                catch
                {
                    try
                    {
                        DateTime dt = DateTime.Parse(DateStr);
                        ret = DateTimeToMinutes(dt);
                    }
                    catch
                    {
                        GlobalVariables.Logger.Log("Date string unknown format : " + DateStr);
                    }
                }
            }
            if (ret == retInitial)
                GlobalVariables.Logger.Log("Date string unknown format : " + DateStr);

            return ret;
        }
    }
    public class DBHandler
    {
        public SQLiteConnection m_dbConnection = null;
        public DBHandler()
        {
            string sql;
            //            SQLiteConnection.CreateFile("EbayCSV.db");
            m_dbConnection = new SQLiteConnection("Data Source=EbayCSV.db;New=False;Version=3;journal_mode=WAL;synchronous=NORMAL");
            m_dbConnection.Open();

            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA cache_size=1000000";
                var name = command.ExecuteScalar();
            }
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA synchronous=OFF";
                var name = command.ExecuteScalar();
            }
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA journal_mode=MEMORY";
                var name = command.ExecuteScalar();
            }
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA temp_store=MEMORY";
                var name = command.ExecuteScalar();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='InventoryCSV'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "InventoryCSV"))
                {

                    sql = "create table InventoryCSV (ebay_id_Hash INT4,asin_hash INT4,vat FLOAT DEFAULT " + GlobalVariables.NULLValue + ", ebay_id_str varchar(20),asin_str varchar(20))";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }

                sql = "CREATE INDEX IF NOT EXISTS UniqueItemId ON InventoryCSV (ebay_id_Hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON InventoryCSV (asin_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueItemIds ON InventoryCSV (ebay_id_str)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductIds ON InventoryCSV (asin_str)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='AMAZON_ORDERS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "AMAZON_ORDERS"))
                {

                    sql = "create table AMAZON_ORDERS (date varchar(20),ORDER_ID varchar(20),TITLE varchar(200),GROSS double,vat double,BuyerName varchar(200),Buyeraddr varchar(200), asin varchar(20), NET double, vat_rate double, asin_hash int4, ORDER_ID_hash int4,TStamp INT,SACCOUNT varchar(200),SELLER varchar(200),SELLER_VAT varchar(200))";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON AMAZON_ORDERS (ORDER_ID_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON AMAZON_ORDERS (asin_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderIds ON AMAZON_ORDERS (ORDER_ID)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductIds ON AMAZON_ORDERS (asin)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='AMAZON_REFUNDS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "AMAZON_REFUNDS"))
                {

                    sql = "create table AMAZON_REFUNDS (date varchar(20),ORDER_ID varchar(20),TITLE varchar(200),GROSS double,vat double,BuyerName varchar(200),Buyeraddr varchar(200), asin varchar(20), NET double, vat_rate double, asin_hash int4, ORDER_ID_hash int4,TStamp INT,SACCOUNT varchar(200),SELLER varchar(200),SELLER_VAT varchar(200))";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON AMAZON_REFUNDS (ORDER_ID_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON AMAZON_REFUNDS (asin_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderIds ON AMAZON_REFUNDS (ORDER_ID)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductIds ON AMAZON_REFUNDS (asin)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='AMAZON_BLOCKED'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "AMAZON_BLOCKED"))
                {
                    sql = "create table AMAZON_BLOCKED (date varchar(20),ORDER_ID varchar(20),DISPATCH varchar(20),TITLE varchar(200),PRICE double,vat double,BuyerName varchar(200),Buyeraddr varchar(200), asin varchar(20),PAYMENT varchar(200), asin_hash int4, ORDER_ID_hash int4,TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON AMAZON_BLOCKED (ORDER_ID_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON AMAZON_BLOCKED (asin_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderIds ON AMAZON_BLOCKED (ORDER_ID)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueProductIds ON AMAZON_BLOCKED (asin)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='PAYPAL_SALES'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "PAYPAL_SALES"))
                {

                    sql = "create table PAYPAL_SALES (date varchar(20),BuyerName varchar(20),GROSS double,PayPalFee double,TransactionID varchar(200),Title varchar(200), ItemId varchar(200), BuyerAddr varchar(200), Phone varchar(200), vat double, net double, vat_rate double, TransactionID_hash int4, TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON PAYPAL_SALES (TransactionID_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderIds ON PAYPAL_SALES (TransactionID)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='PAYPAL_REFUNDS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "PAYPAL_REFUNDS"))
                {

                    sql = "create table PAYPAL_REFUNDS (date varchar(20),BuyerName varchar(20),GROSS double,PayPalFee double,TransactionID varchar(200),Title varchar(200), ItemId varchar(200), ReferenceID varchar(200), vat double, net double, vat_rate double, TransactionID_hash int4, ReferenceID_hash int4, TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON PAYPAL_REFUNDS (TransactionID_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderIds ON PAYPAL_REFUNDS (TransactionID)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='EBAY_SALES'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "EBAY_SALES"))
                {

                    sql = "create table EBAY_SALES (date varchar(20),BuyerName varchar(20),address varchar(20),TransactionID varchar(200), ItemId varchar(200),GROSS double, vat double, net double, vat_rate double, Fee double, FeeNet double, FeeVAT double, TransactionID_hash int4, TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderId ON EBAY_SALES (TransactionID_hash)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();

                sql = "CREATE INDEX IF NOT EXISTS UniqueOrderIds ON EBAY_SALES (TransactionID)";
                command = new SQLiteCommand(sql, m_dbConnection);
                command.ExecuteNonQuery();
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='EBAY_REFUNDS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "EBAY_REFUNDS"))
                {

                    sql = "create table EBAY_REFUNDS (OrderType varchar(50),date varchar(20),BuyerName varchar(20),address varchar(20),TransactionID varchar(200), ItemId varchar(200),GROSS double, vat double, net double, vat_rate double, Fee double, FeeNet double, FeeVAT double, TransactionID_hash int4, TStamp INT)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }
        }

        public SQLiteConnection GetConnection()
        {
            return m_dbConnection;
        }

        private void ReplaceInventoryRow(string ebay_id, string asin, string vat)
        {
            int ebayhash = ebay_id.GetHashCode();
            int asinhash = asin.GetHashCode();

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE INTO InventoryCSV(ebay_id_Hash,asin_hash,vat,ebay_id_str,asin_str) VALUES(@ebay_id_Hash, @asin_hash, @vat,@ebay_id_str,@asin_str)";

            cmd.Parameters.AddWithValue("@ebay_id_Hash", ebayhash);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@vat", double.Parse(vat));
            cmd.Parameters.AddWithValue("@ebay_id_str", ebay_id);
            cmd.Parameters.AddWithValue("@asin_str", asin);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }
        public enum InventoryInsertResultCodes
        {
            RowExistedButWasEmpty = 1,
            RowExisted = 2,
            RowDidNotExistInsertedNew = 3,
            RowInvalidValues = 4,
            RowDidNotExist = 5,
            RowDeleted = 6,
        }
        public InventoryInsertResultCodes InsertInventory(string ebay_id, string asin, string vat)
        {
            InventoryInsertResultCodes ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            int ebayhash = ebay_id.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ebay_id_Hash,asin_str,vat FROM InventoryCSV where ebay_id_Hash=@ebayhash and ebay_id_str=@ebay_id";
            cmd.Parameters.AddWithValue("@ebayhash", ebayhash);
            cmd.Parameters.AddWithValue("@ebay_id", ebay_id);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                //check if the existing row has values
                int ebayidhash = rdr.GetInt32(0);
                string asin_str = "";
                if (!rdr.IsDBNull(1))
                    asin_str = rdr.GetString(1);
                double oldVAT = rdr.GetDouble(2);
                ReturnCode = InventoryInsertResultCodes.RowExisted;
                if (asin_str.Length == 0 && asin.Length != 0)
                {
                    //value exists, but it still needs import
                    UpdateInventoryEbayAsin(ebay_id, asin);
                    ReturnCode = InventoryInsertResultCodes.RowExistedButWasEmpty;
                }
                if (oldVAT == (double)GlobalVariables.NULLValue && double.Parse(vat) != (double)GlobalVariables.NULLValue)
                {
                    //value exists, but it still needs import
                    UpdateInventoryVatEbay(ebay_id, vat);
                    ReturnCode = InventoryInsertResultCodes.RowExistedButWasEmpty;
                }
            }
            else
            {
                ReplaceInventoryRow(ebay_id, asin, vat);
                ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;/**/
            }

            return ReturnCode;
        }

        private void ClearTable(string Table)
        {
            if (m_dbConnection == null)
                return;
            if (Table == null || Table.Length == 0)
                return;
            string sql = "delete from " + Table;
            SQLiteCommand command = new SQLiteCommand(sql, m_dbConnection);
            command.ExecuteNonQuery();
        }

        public void ClearInventory()
        {
            GlobalVariables.Logger.Log("Content of the Inventory table has been cleared");
            ClearTable("InventoryCSV");
        }

        ~DBHandler()
        {
            if (m_dbConnection != null)
                m_dbConnection = null;
        }

        public void UpdateInventoryVatEbay(string ebay_id, string vat)
        {
            if (ebay_id == null || ebay_id.Length == 0 || vat == null || vat.Length == 0)
                return;

            int ebayhash = ebay_id.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT 1 FROM InventoryCSV where ebay_id_Hash=@ebayhash and ebay_id_str=@ebay_id";
            cmd.Parameters.AddWithValue("@ebayhash", ebayhash);
            cmd.Parameters.AddWithValue("@ebay_id", ebay_id);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                var cmd2 = new SQLiteCommand(m_dbConnection);
                cmd2.CommandText = "UPDATE InventoryCSV set vat=@vat where ebay_id_Hash=@ebayhash and ebay_id_str=@ebay_id";
                cmd2.Parameters.AddWithValue("@ebayhash", ebayhash);
                cmd2.Parameters.AddWithValue("@vat", double.Parse(vat));
                cmd2.Parameters.AddWithValue("@ebay_id", ebay_id);
                cmd2.Prepare();
                cmd2.ExecuteNonQuery();
            }
            else
            {
                var cmd2 = new SQLiteCommand(m_dbConnection);
                cmd2.CommandText = "INSERT into InventoryCSV(ebay_id_Hash,vat,ebay_id_str)values(@ebayhash,@vat,@ebay_id)";
                cmd2.Parameters.AddWithValue("@ebayhash", ebayhash);
                cmd2.Parameters.AddWithValue("@vat", double.Parse(vat));
                cmd2.Parameters.AddWithValue("@ebay_id", ebay_id);
                cmd2.Prepare();
                cmd2.ExecuteNonQuery();
            }
        }

        public void UpdateInventoryVatAsin(string asin, string vat)
        {
            if (asin == null || asin.Length == 0 || vat == null || vat.Length == 0)
                return;

            int asinHash = asin.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "update InventoryCSV set vat=@vat where asin_hash=@asin_hash and asin_str=@asin_str";
            cmd.Parameters.AddWithValue("@vat", double.Parse(vat));
            cmd.Parameters.AddWithValue("@asin_hash", asinHash);
            cmd.Parameters.AddWithValue("@asin_str", asin);
            cmd.Prepare();
            cmd.ExecuteNonQuery();
        }

        public void UpdateInventoryEbayAsin(string ebay_id, string asin)
        {
            if (asin == null || asin.Length == 0 || ebay_id == null || ebay_id.Length == 0)
                return;

            int ebayhash = ebay_id.GetHashCode();
            int asinHash = asin.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "update InventoryCSV set asin_hash=@asin_hash,asin_str=@asin_str where ebay_id_hash=@ebay_id_hash and ebay_id_str=@ebay_id_str and (asin_hash=0 or asin_hash is NULL)";
            cmd.Parameters.AddWithValue("@asin_hash", asinHash);
            cmd.Parameters.AddWithValue("@asin_str", asin);
            cmd.Parameters.AddWithValue("@ebay_id_hash", ebayhash);
            cmd.Parameters.AddWithValue("@ebay_id_str", ebay_id);
            cmd.Prepare();
            cmd.ExecuteNonQuery();
        }

        public void ExportInventoryTable()
        {
            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateInventoryRunFile("./reports/INVENTORY.csv");
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ebay_id_str,asin_str,vat FROM InventoryCSV";
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                string ebay_id_str = "";
                if (!rdr.IsDBNull(0))
                    ebay_id_str = rdr.GetString(0);
                string asin_str = "";
                if (!rdr.IsDBNull(1))
                    asin_str = rdr.GetString(1);
                double vat = rdr.GetDouble(2);
                ExportInventoryCSV.InventoryRunFileAddRow(ebay_id_str, asin_str, vat);
            }
            ExportInventoryCSV.Dispose();
        }

        public void UpdateAllMissingInventoryRows(string ASINCol, string VATCol)
        {
            int asinhash = ASINCol.GetHashCode();
            double VAT = double.Parse(VATCol);

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "update InventoryCSV set vat=@VAT where asin_hash=@asin_hash and vat=" + GlobalVariables.NULLValue + " and asin_str=@asin";

            cmd.Parameters.AddWithValue("@VAT", VAT);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@asin", ASINCol);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }
        public InventoryInsertResultCodes InsertAmazonOrder(string TableName, string DateCol, string IdCol, string TitleCol, string PriceCol, string VATCol, string BuyerCol, string AddressCol, string ASINCol, double NET, double VAT_RATE, string SACCOUNT, string SELLER, string SELLER_VAT)
        {
            InventoryInsertResultCodes ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            int ORDER_ID_hashhash = IdCol.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT 1 FROM " + TableName + " where ORDER_ID_hash=@ORDER_ID_hash and order_id=@order_id";
            cmd.Parameters.AddWithValue("@ORDER_ID_hash", ORDER_ID_hashhash);
            cmd.Parameters.AddWithValue("@order_id", IdCol);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                ReturnCode = InventoryInsertResultCodes.RowExisted;
            }
            else
            {
                int IdColhash = IdCol.GetHashCode();
                int asinhash = ASINCol.GetHashCode();
                long TStamp = DateParser.GetUnixStamp(DateCol);

                var cmd2 = new SQLiteCommand(m_dbConnection);
                cmd2.CommandText = "REPLACE INTO " + TableName + "(date,ORDER_ID,TITLE,GROSS,vat,BuyerName,Buyeraddr,asin,NET,vat_rate,asin_hash,ORDER_ID_hash,TStamp,SACCOUNT,SELLER,SELLER_VAT) " +
                    "VALUES(@date,@ORDER_ID,@TITLE,@GROSS,@vat,@BuyerName,@Buyeraddr,@asin,@NET,@vat_rate,@asin_hash,@ORDER_ID_hash,@TStamp,@SACCOUNT,@SELLER,@SELLER_VAT)";

                cmd2.Parameters.AddWithValue("@date", DateCol);
                cmd2.Parameters.AddWithValue("@ORDER_ID", IdCol);
                cmd2.Parameters.AddWithValue("@TITLE", TitleCol);
                cmd2.Parameters.AddWithValue("@GROSS", PriceCol);
                cmd2.Parameters.AddWithValue("@vat", double.Parse(VATCol));
                cmd2.Parameters.AddWithValue("@BuyerName", BuyerCol);
                cmd2.Parameters.AddWithValue("@Buyeraddr", AddressCol);
                cmd2.Parameters.AddWithValue("@asin", ASINCol);
                cmd2.Parameters.AddWithValue("@NET", NET);
                cmd2.Parameters.AddWithValue("@vat_rate", VAT_RATE);
                cmd2.Parameters.AddWithValue("@asin_hash", asinhash);
                cmd2.Parameters.AddWithValue("@ORDER_ID_hash", IdColhash);
                cmd2.Parameters.AddWithValue("@TStamp", TStamp);
                cmd2.Parameters.AddWithValue("@SACCOUNT", SACCOUNT);
                cmd2.Parameters.AddWithValue("@SELLER", SELLER);
                cmd2.Parameters.AddWithValue("@SELLER_VAT", SELLER_VAT);
                cmd2.Prepare();

                cmd2.ExecuteNonQuery();

                ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            }

            return ReturnCode;
        }

        public InventoryInsertResultCodes IsAmazonOrderBlocked(string IdCol)
        {
            int ORDER_ID_hashhash = IdCol.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT 1 FROM AMAZON_BLOCKED where ORDER_ID_hash=@ORDER_ID_hash and order_id=@order_id";
            cmd.Parameters.AddWithValue("@ORDER_ID_hash", ORDER_ID_hashhash);
            cmd.Parameters.AddWithValue("@order_id", IdCol);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
                return InventoryInsertResultCodes.RowExisted;
            return InventoryInsertResultCodes.RowDidNotExist;
        }

        public void DeleteAmazonOrderBlocked(string IdCol)
        {
            int ORDER_ID_hashhash = IdCol.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "Delete FROM AMAZON_BLOCKED where ORDER_ID_hash=@ORDER_ID_hash and order_id=@order_id";
            cmd.Parameters.AddWithValue("@ORDER_ID_hash", ORDER_ID_hashhash);
            cmd.Parameters.AddWithValue("@order_id", IdCol);
            cmd.Prepare();
            cmd.ExecuteNonQuery();
        }

        public InventoryInsertResultCodes InsertAmazonBlocked(string TableName, string IdCol, string DispatchedCol, string DateCol, string TitleCol, string PriceCol, string VatCol, string BuyerCol, string AddressCol, string ASINCol, string PaymentCol)
        {
            InventoryInsertResultCodes ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            //check if ts record already exists
            InventoryInsertResultCodes AmazonOrderBlockedRowStatus = IsAmazonOrderBlocked(IdCol);
            if (AmazonOrderBlockedRowStatus == InventoryInsertResultCodes.RowExisted)
            {
                return InventoryInsertResultCodes.RowExisted;
            }
            else
            {
                int IdColhash = IdCol.GetHashCode();
                int asinhash = ASINCol.GetHashCode();
                long TStamp = DateParser.GetUnixStamp(DateCol);

                var cmd2 = new SQLiteCommand(m_dbConnection);
                cmd2.CommandText = "INSERT INTO " + TableName + "(date,ORDER_ID,DISPATCH,TITLE,PRICE,vat,BuyerName,Buyeraddr,asin,PAYMENT,asin_hash,ORDER_ID_hash,TStamp) " +
                    "VALUES(@date,@ORDER_ID,@DISPATCH,@TITLE,@PRICE,@vat,@BuyerName,@Buyeraddr,@asin,@PAYMENT,@asin_hash,@ORDER_ID_hash,@TStamp)";

                cmd2.Parameters.AddWithValue("@date", DateCol);
                cmd2.Parameters.AddWithValue("@ORDER_ID", IdCol);
                cmd2.Parameters.AddWithValue("@DISPATCH", DispatchedCol);
                cmd2.Parameters.AddWithValue("@TITLE", TitleCol);
                cmd2.Parameters.AddWithValue("@PRICE", PriceCol);
                cmd2.Parameters.AddWithValue("@vat", double.Parse(VatCol));
                cmd2.Parameters.AddWithValue("@BuyerName", BuyerCol);
                cmd2.Parameters.AddWithValue("@Buyeraddr", AddressCol);
                cmd2.Parameters.AddWithValue("@asin", ASINCol);
                cmd2.Parameters.AddWithValue("@PAYMENT", PaymentCol);
                cmd2.Parameters.AddWithValue("@asin_hash", asinhash);
                cmd2.Parameters.AddWithValue("@ORDER_ID_hash", IdColhash);
                cmd2.Parameters.AddWithValue("@TStamp", TStamp);
                cmd2.Prepare();

                cmd2.ExecuteNonQuery();

                ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            }
            return ReturnCode;
        }

        public void ClearAmazonBlocked()
        {
            ClearTable("AMAZON_BLOCKED");
            GlobalVariables.Logger.Log("Content of the AMAZON-BLOCKED table has been cleared");
        }

        public void ClearEbaySales()
        {
            ClearTable("EBAY_SALES");
            GlobalVariables.Logger.Log("Content of the EBAY-SALES table has been cleared");
        }

        public void ClearEbayRefunds()
        {
            ClearTable("EBAY_REFUNDS");
            GlobalVariables.Logger.Log("Content of the EBAY-REFUNDS table has been cleared");
        }

        public InventoryInsertResultCodes DeleteAmazonOrder(string IdCol)
        {
            InventoryInsertResultCodes ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            //check if ts record already exists
            InventoryInsertResultCodes AmazonOrderBlockedRowStatus = IsAmazonOrderBlocked(IdCol);
            if (AmazonOrderBlockedRowStatus == InventoryInsertResultCodes.RowExisted)
            {
                var cmd2 = new SQLiteCommand(m_dbConnection);
                cmd2.CommandText = "delete from AMAZON_ORDERS where ORDER_ID=@ORDER_ID";
                cmd2.Parameters.AddWithValue("@ORDER_ID", IdCol);
                cmd2.Prepare();
                cmd2.ExecuteNonQuery();
                ReturnCode = InventoryInsertResultCodes.RowDeleted;
            }
            else
            {
                ReturnCode = InventoryInsertResultCodes.RowDidNotExist;
            }
            return ReturnCode;
        }

        public void ClearAmazonOrders()
        {
            ClearTable("AMAZON_ORDERS");
            GlobalVariables.Logger.Log("Content of the AMAZON-ORDERS table has been cleared");
        }
        public void ClearAmazonRefunds()
        {
            ClearTable("AMAZON_REFUNDS");
            GlobalVariables.Logger.Log("Content of the AMAZON-REFUNDS table has been cleared");
        }
        public void ExportAmazonOrdersTable(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/" + CSVFileName + ".csv");
            ExportInventoryCSV.WriteLine("date,ORDER_ID,TITLE,GROSS,vat,BuyerName,Buyeraddr,asin,NET,vat_rate,SACCOUNT,SELLER,SELLER_VAT\n");
            dynamic record = new System.Dynamic.ExpandoObject();
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,ORDER_ID,TITLE,GROSS,vat,BuyerName,Buyeraddr,asin,NET,vat_rate,SACCOUNT,SELLER,SELLER_VAT FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                record.date = "";
                if (!rdr.IsDBNull(0))
                    record.date = rdr.GetString(0);
                record.ORDER_ID = "";
                if (!rdr.IsDBNull(1))
                    record.ORDER_ID = rdr.GetString(1);
                record.TITLE = "";
                if (!rdr.IsDBNull(2))
                    record.TITLE = rdr.GetString(2);
                record.GROSS = Math.Round(rdr.GetDouble(3), 2).ToString();
                record.vat = Math.Round(rdr.GetDouble(4), 2).ToString();
                record.Name = "";
                if (!rdr.IsDBNull(5))
                    record.Name = rdr.GetString(5);
                record.ADDRESS = "";
                if (!rdr.IsDBNull(6))
                    record.ADDRESS = rdr.GetString(6);
                record.asin = "";
                if (!rdr.IsDBNull(7))
                    record.asin = rdr.GetString(7);
                double NET = rdr.GetDouble(8);
                if (NET != (double)GlobalVariables.NULLValue)
                {
                    record.NET = Math.Round(NET, 2).ToString();
                }
                else
                    record.NET = "";
                double vat_rate = rdr.GetDouble(9);
                if (vat_rate != (double)GlobalVariables.NULLValue)
                    record.vat_rate = Math.Round(vat_rate, 2).ToString();
                else
                    record.vat_rate = "";
                record.ACCOUNT = "";
                if (!rdr.IsDBNull(10))
                    record.ACCOUNT = rdr.GetString(10);
                record.SELLER = "";
                if (!rdr.IsDBNull(11))
                    record.SELLER = rdr.GetString(11);
                record.SELLER_VAT = "";
                if (!rdr.IsDBNull(12))
                    record.SELLER_VAT = rdr.GetString(12);

                ExportInventoryCSV.WriteDynamicFileRow(record);
            }
            /*            if (ExportInventoryCSV.RowsWritten() == 0)
                        {
                            record.date = "";
                            record.ORDER_ID = "";
                            record.TITLE = "";
                            record.GROSS = 0;
                            record.vat = 0;
                            record.Name = "";
                            record.ADDRESS = "";
                            record.asin = "";
                            record.NET = "";
                            record.vat_rate = "";
                            record.ACCOUNT = "";
                            record.SELLER = "";
                            record.SELLER_VAT = "";
                            ExportInventoryCSV.WriteDynamicFileHeader(record);
                        }*/
            ExportInventoryCSV.Dispose();
        }
        public void ClearPaypalSales()
        {
            ClearTable("PAYPAL_SALES");
            GlobalVariables.Logger.Log("Content of the PAYPAL-SALES table has been cleared");
        }
        public void ClearPaypalRefunds()
        {
            ClearTable("PAYPAL_REFUNDS");
            GlobalVariables.Logger.Log("Content of the PAYPAL-REFUNDS table has been cleared");
        }

        public InventoryInsertResultCodes InsertPaypalRow(string DateCol, string NameCol, string PriceCol, string PPFeeCol, string TransactionIDCol, string TitleCol, string ItemIdCol, string AddressCol, string PhoneCol)
        {
            InventoryInsertResultCodes ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            int IdColhash = TransactionIDCol.GetHashCode();

            string TableName = "PAYPAL_SALES";

#if USE_CACHING
            if (GlobalVariables.CachedIndexes.CheckRowExists(IdColhash, TransactionIDCol)==true)
            {
                ReturnCode = InventoryInsertResultCodes.RowExisted;
            }
            else
            {
                GlobalVariables.CachedIndexes.AddRow(IdColhash, TransactionIDCol);
#else
            //check if ts record already exists
            var cmd1 = new SQLiteCommand(m_dbConnection);
            cmd1.CommandText = "SELECT 1 FROM " + TableName + " where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
            cmd1.Parameters.AddWithValue("@IdColhash", IdColhash);
            cmd1.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
            cmd1.Prepare();

            SQLiteDataReader rdr = cmd1.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                ReturnCode = InventoryInsertResultCodes.RowExisted;
            }
            else
            {
#endif

                long TStamp = DateParser.GetUnixStamp(DateCol);
                var cmd = new SQLiteCommand(m_dbConnection);
                cmd.CommandText = "REPLACE INTO " + TableName + "(date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,BuyerAddr,Phone,vat,net,vat_rate,TransactionID_hash,TStamp) " +
                    "VALUES(@DateCol,@NameCol,@PriceCol,@PPFeeCol,@TransactionIDCol,@TitleCol,@ItemIdCol,@AddressCol,@PhoneCol,@vat,@net,@vat_rate,@TransactionID_hash,@TStamp)";

                cmd.Parameters.AddWithValue("@DateCol", DateCol);
                cmd.Parameters.AddWithValue("@NameCol", NameCol);
                cmd.Parameters.AddWithValue("@PriceCol", double.Parse(PriceCol));
                cmd.Parameters.AddWithValue("@PPFeeCol", double.Parse(PPFeeCol));
                cmd.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
                cmd.Parameters.AddWithValue("@TitleCol", TitleCol);
                cmd.Parameters.AddWithValue("@ItemIdCol", ItemIdCol);
                cmd.Parameters.AddWithValue("@AddressCol", AddressCol);
                cmd.Parameters.AddWithValue("@PhoneCol", PhoneCol);
                cmd.Parameters.AddWithValue("@vat", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@net", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@vat_rate", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@TransactionID_hash", IdColhash);
                cmd.Parameters.AddWithValue("@TStamp", TStamp);
                cmd.Prepare();

                cmd.ExecuteNonQuery();

                ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            }

            return ReturnCode;
        }

        public InventoryInsertResultCodes InsertPaypalRefundRow(string DateCol, string NameCol, string PriceCol, string PPFeeCol, string TransactionIDCol, string TitleCol, string ItemIdCol, string ReferenceIDCol)
        {
            InventoryInsertResultCodes ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            int IdColhash = TransactionIDCol.GetHashCode();

            string TableName = "PAYPAL_REFUNDS";

            //check if ts record already exists
            var cmd1 = new SQLiteCommand(m_dbConnection);
            cmd1.CommandText = "SELECT 1 FROM " + TableName + " where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
            cmd1.Parameters.AddWithValue("@IdColhash", IdColhash);
            cmd1.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
            cmd1.Prepare();

            SQLiteDataReader rdr = cmd1.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                ReturnCode = InventoryInsertResultCodes.RowExisted;
            }
            else
            {
                long TStamp = DateParser.GetUnixStamp(DateCol);
                var cmd = new SQLiteCommand(m_dbConnection);
                cmd.CommandText = "REPLACE INTO " + TableName + "(date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,ReferenceID,vat,net,vat_rate,TransactionID_hash,TStamp) " +
                    "VALUES(@DateCol,@NameCol,@PriceCol,@PPFeeCol,@TransactionIDCol,@TitleCol,@ItemIdCol,@ReferenceID,@vat,@net,@vat_rate,@TransactionID_hash,@TStamp)";

                cmd.Parameters.AddWithValue("@DateCol", DateCol);
                cmd.Parameters.AddWithValue("@NameCol", NameCol);
                cmd.Parameters.AddWithValue("@PriceCol", double.Parse(PriceCol));
                cmd.Parameters.AddWithValue("@PPFeeCol", double.Parse(PPFeeCol));
                cmd.Parameters.AddWithValue("@TransactionIDCol", TransactionIDCol);
                cmd.Parameters.AddWithValue("@TitleCol", TitleCol);
                cmd.Parameters.AddWithValue("@ItemIdCol", ItemIdCol);
                cmd.Parameters.AddWithValue("@ReferenceID", ReferenceIDCol);
                cmd.Parameters.AddWithValue("@vat", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@net", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@vat_rate", GlobalVariables.NULLValue);
                cmd.Parameters.AddWithValue("@TransactionID_hash", IdColhash);
                cmd.Parameters.AddWithValue("@TStamp", TStamp);
                cmd.Prepare();

                cmd.ExecuteNonQuery();

                ReturnCode = InventoryInsertResultCodes.RowDidNotExistInsertedNew;
            }

            return ReturnCode;
        }
        public void ExportPaypalSales(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/" + CSVFileName + ".csv");
            var cmd = new SQLiteCommand(m_dbConnection);
            if (TableName == "PAYPAL_SALES")
            {
                cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,BuyerAddr,Phone,vat,net,vat_rate,tstamp FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
                ExportInventoryCSV.WriteLine("date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,BuyerAddr,Phone,vat,net,vat_rate\n");
            }
            else
            {
                cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,ReferenceID,1,vat,net,vat_rate FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
                ExportInventoryCSV.WriteLine("date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,ReferenceID,vat,net,vat_rate\n");
            }
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            dynamic record = new System.Dynamic.ExpandoObject();
            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                if (!rdr.IsDBNull(0))
                    record.Date = rdr.GetString(0);
                else
                    record.Date = "";
                if (!rdr.IsDBNull(1))
                    record.Name = rdr.GetString(1);
                else
                    record.Name = "";
                double Gross = rdr.GetDouble(2);
                if (Gross != (double)GlobalVariables.NULLValue)
                    record.Gross = Gross.ToString();
                else
                    record.Gross = "";
                double Fee = rdr.GetDouble(3);
                if (Fee != (double)GlobalVariables.NULLValue)
                    record.Paypal_Fee = Math.Round(Fee, 2).ToString();
                else
                    record.Paypal_Fee = "";
                if (!rdr.IsDBNull(4))
                    record.Transaction_ID = rdr.GetString(4);
                else
                    record.Transaction_ID = "";
                if (!rdr.IsDBNull(5))
                    record.Title = rdr.GetString(5);
                else
                    record.Title = "";
                if (!rdr.IsDBNull(6))
                    record.Item_Id = rdr.GetString(6);
                else
                    record.Item_Id = "";
                if (TableName == "PAYPAL_SALES")
                {
                    if (!rdr.IsDBNull(7))
                        record.Address = rdr.GetString(7);
                    else
                        record.Address = "";
                    if (!rdr.IsDBNull(8))
                        record.Phone = rdr.GetString(8);
                    else
                        record.Phone = "";
                }
                else
                {
                    if (!rdr.IsDBNull(7))
                        record.Reference_Id = rdr.GetString(7);
                    else
                        record.Reference_Id = "";
                }
                double vat = rdr.GetDouble(9);
                if (vat != (double)GlobalVariables.NULLValue)
                    record.Vat = vat.ToString();
                else
                    record.Vat = "";
                double NET = rdr.GetDouble(10);
                if (NET != (double)GlobalVariables.NULLValue)
                    record.NET = Math.Round(NET, 2).ToString();
                else
                    record.NET = "";
                double vat_rate = rdr.GetDouble(11);
                if (vat_rate != (double)GlobalVariables.NULLValue)
                    record.vat_rate = Math.Round(vat_rate, 2).ToString();
                else
                    record.vat_rate = "";

                //                long unixTimeStamp = rdr.GetInt64(12);
                //                System.DateTime dtDateTime = new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Utc);
                //                dtDateTime = dtDateTime.AddMinutes(unixTimeStamp).ToLocalTime();
                //                record.tstamp = dtDateTime.ToString();/**/

                ExportInventoryCSV.WriteDynamicFileRow(record);
            }
/*            if (ExportInventoryCSV.RowsWritten() == 0)
            {
                record.Date = "";
                record.Name = "";
                record.Gross = "";
                record.Paypal_Fee = "";
                record.Transaction_ID = "";
                record.Title = "";
                record.Item_Id = "";
                if (TableName == "PAYPAL_SALES")
                {
                    record.Address = "";
                    record.Phone = "";
                }
                else
                {
                    record.Reference_Id = "";
                }
                record.Vat = "";
                record.NET = "";
                record.vat_rate = "";
                ExportInventoryCSV.WriteDynamicFileHeader(record);
            }*/
            ExportInventoryCSV.Dispose();
        }

        public double FloatSubstract(double a, double b, int Precision)
        {
            //because 6.71 and not 6.709999
            string astr = a.ToString();
            string bstr = b.ToString();
            string[] aparts = astr.Split('.');
            string[] bparts = bstr.Split('.');
            long A = long.Parse(aparts[0]);
            long B = long.Parse(bparts[0]);
            long SignA = 1;
            long SignB = 1;
            if (a < 0)
                SignA = -1;
            if (a < 0)
                SignB = -1;
            for (int i = 0; i < Precision; i++)
            {
                A = A * 10;
                if (aparts.Length > 1 && aparts[1].Length > i)
                    A = A + SignA * (aparts[1][i] - '0');
                B = B * 10;
                if (bparts.Length > 1 && bparts[1].Length > i)
                    B = B + SignB * (bparts[1][i] - '0');
            }
            double res = (double)(A - B);
            for (int i = 0; i < Precision; i++)
                res = res / 10;
            return (double)res;
        }

        public void UpdateVAT()
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }
            GlobalVariables.Logger.Log("Update VAT - started");
            GlobalVariables.ImportingToDBBlock = "UpdateVat";
            WriteCSVFile ExportSalesCSV = new WriteCSVFile();
            ExportSalesCSV.CreateDynamicFile("./reports/no-vat-paypal-sales.csv");
            WriteCSVFile ExportRefundsCSV = new WriteCSVFile();
            ExportRefundsCSV.CreateDynamicFile("./reports/no-vat-paypal-refunds.csv");
            ExportSalesCSV.WriteLine("Date,Name,Gross,Paypal_Fee,Transaction_ID,Title,Item_Id,Address,Phone,Vat,NET,vat_rate\n");
            ExportRefundsCSV.WriteLine("Date,Name,Gross,Paypal_Fee,Transaction_ID,Title,Item_Id,Address,Phone,Vat,NET,vat_rate\n");
            for (int i = 0; i < 2; i++)
            {
                var cmd = new SQLiteCommand(m_dbConnection);
                if (i == 0)
                    cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,BuyerAddr,Phone,vat,net,vat_rate FROM PAYPAL_SALES";
                else
                    cmd.CommandText = "SELECT date,BuyerName,GROSS,PayPalFee,TransactionID,Title,ItemId,ReferenceID,1,vat,net,vat_rate FROM PAYPAL_REFUNDS";
                cmd.Prepare();

                dynamic record = new System.Dynamic.ExpandoObject();
                SQLiteDataReader rdr = cmd.ExecuteReader();
                while (rdr.Read() && rdr.HasRows == true)
                {
                    //for refunds table, we do not process rows that already have a value
                    double vat_rate = rdr.GetDouble(11);
                    if (i == 1 && vat_rate >= 0)
                        continue;

                    //get the itemID
                    string EbayId = "";
                    if (!rdr.IsDBNull(6))
                        EbayId = rdr.GetString(6);

                    //check if inventory has this value
                    int ebayhash = EbayId.GetHashCode();
                    var cmd1 = new SQLiteCommand(m_dbConnection);
                    cmd1.CommandText = "SELECT vat FROM InventoryCSV where ebay_id_Hash=@ebay_id_Hash and ebay_id_str=@ebay_id_str limit 1";
                    cmd1.Parameters.AddWithValue("@ebay_id_Hash", ebayhash);
                    cmd1.Parameters.AddWithValue("@ebay_id_str", EbayId);
                    cmd1.Prepare();
                    SQLiteDataReader rdr2 = cmd1.ExecuteReader();
                    double InventoryVAT = GlobalVariables.NULLValue;
                    if (rdr2.Read() && rdr2.HasRows == true)
                        InventoryVAT = rdr2.GetDouble(0);

                    if (InventoryVAT == GlobalVariables.NULLValue)
                    {
                        if (!rdr.IsDBNull(0))
                            record.Date = rdr.GetString(0);
                        else
                            record.Date = "";
                        if (!rdr.IsDBNull(1))
                            record.Name = rdr.GetString(1);
                        else
                            record.Name = "";
                        double Gross = rdr.GetDouble(2);
                        if (Gross != (double)GlobalVariables.NULLValue)
                            record.Gross = Math.Round(Gross, 2).ToString();
                        else
                            record.Gross = "";
                        double Fee = rdr.GetDouble(3);
                        if (Fee != (double)GlobalVariables.NULLValue)
                            record.Paypal_Fee = Math.Round(Fee, 2).ToString();
                        else
                            record.Paypal_Fee = "";
                        if (!rdr.IsDBNull(4))
                            record.Transaction_ID = rdr.GetString(4);
                        else
                            record.Transaction_ID = "";
                        if (!rdr.IsDBNull(5))
                            record.Title = rdr.GetString(5);
                        else
                            record.Title = "";
                        if (!rdr.IsDBNull(6))
                            record.Item_Id = rdr.GetString(6);
                        else
                            record.Item_Id = "";
                        if (i == 0)
                        {
                            if (!rdr.IsDBNull(7))
                                record.Address = rdr.GetString(7);
                            else
                                record.Address = "";
                            if (!rdr.IsDBNull(8))
                                record.Phone = rdr.GetString(8);
                            else
                                record.Phone = "";
                        }
                        else
                        {
                            if (!rdr.IsDBNull(7))
                                record.Reference_Id = rdr.GetString(7);
                            else
                                record.Reference_Id = "";
                        }
                        double vat = rdr.GetDouble(9);
                        if (vat != (double)GlobalVariables.NULLValue)
                            record.Vat = vat.ToString();
                        else
                            record.Vat = "";
                        double NET = rdr.GetDouble(10);
                        if (NET != (double)GlobalVariables.NULLValue)
                            record.NET = Math.Round(NET, 2).ToString();
                        else
                            record.NET = "";
                        //                        double vat_rate = rdr.GetDouble(11);
                        if (vat_rate != (double)GlobalVariables.NULLValue)
                            record.vat_rate = vat_rate.ToString();
                        else
                            record.vat_rate = "";
                        //write it to the csv
                        if (i == 0)
                            ExportSalesCSV.WriteDynamicFileRow(record);
                        else
                            ExportRefundsCSV.WriteDynamicFileRow(record);
                    }
                    //we can update vat if we got here
                    else
                    {
                        string Transaction_ID = "";
                        if (!rdr.IsDBNull(4))
                            Transaction_ID = rdr.GetString(4);

                        int Transaction_ID_Hash = Transaction_ID.GetHashCode();
                        double Gross = rdr.GetDouble(2);
                        double NET = Gross / (1 + InventoryVAT / 100);
                        NET = Math.Round(NET, 2);
                        double vat = FloatSubstract(Gross, NET, 3);
                        //                        double vat = Gross - NET;
                        //                        vat = System.Math.Ceiling(vat * 1000) / 1000;

                        var cmd2 = new SQLiteCommand(m_dbConnection);
                        if (i == 0)
                            cmd2.CommandText = "UPDATE PAYPAL_SALES set vat_rate=@vat_rate,net=@net,vat=@vat where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
                        else
                            cmd2.CommandText = "UPDATE PAYPAL_REFUNDS set vat_rate=@vat_rate,net=@net,vat=@vat where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
                        cmd2.Parameters.AddWithValue("@vat_rate", InventoryVAT);
                        cmd2.Parameters.AddWithValue("@net", NET);
                        cmd2.Parameters.AddWithValue("@vat", vat);
                        cmd2.Parameters.AddWithValue("@IdColhash", Transaction_ID_Hash);
                        cmd2.Parameters.AddWithValue("@TransactionIDCol", Transaction_ID);
                        cmd2.Prepare();
                        cmd2.ExecuteNonQuery();
                    }
                }
            }
            GlobalVariables.ImportingToDBBlock = "";
            ExportSalesCSV.Dispose();
            ExportRefundsCSV.Dispose();
            GlobalVariables.Logger.Log("Update VAT - Finished");
        }

        public void ExportAccountingSalesReport(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/" + CSVFileName + ".csv");
            ExportInventoryCSV.WriteLine("DATE,TYPE,DETAILS,CURRENCY,GROSS,VAT,NET,VAT_RATE,FEES_GROSS,FEES_VAT,FEES_NET\n");

            dynamic record = new System.Dynamic.ExpandoObject();

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,TransactionID,ItemId,GROSS,VAT,NET,vat_rate,PayPalFee FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                if (!rdr.IsDBNull(0))
                    record.DATE = rdr.GetString(0);
                else
                    record.DATE = "";

                if (!rdr.IsDBNull(1))
                    record.Type = rdr.GetString(1);
                else
                    record.Type = "";

                if (!rdr.IsDBNull(2))
                    record.DETAILS = rdr.GetString(2);
                else
                    record.DETAILS = "";

                record.CURRENCY = "GBP";

                double Gross = rdr.GetDouble(3);
                if (Gross != (double)GlobalVariables.NULLValue)
                    record.GROSS = Math.Round(Gross, 2).ToString();
                else
                    record.GROSS = "";

                double vat = rdr.GetDouble(4);
                if (vat != (double)GlobalVariables.NULLValue)
                    record.VAT = Math.Round(vat, 2).ToString();
                else
                    record.Vat = "";

                double NET = rdr.GetDouble(5);
                if (NET != (double)GlobalVariables.NULLValue)
                    record.NET = Math.Round(NET, 2).ToString();
                else
                    record.NET = "";

                double vat_rate = rdr.GetDouble(6);
                if (vat_rate != (double)GlobalVariables.NULLValue)
                    record.VAT_RATE = Math.Round(vat_rate, 2).ToString();
                else
                    record.VAT_RATE = "";

                double Fee = rdr.GetDouble(7);
                record.fee = 0;

                record.FEES_VAT = 0.00;

                if (Fee != (double)GlobalVariables.NULLValue)
                    record.FEES_NET = Math.Round(Fee, 2).ToString();
                else
                    record.FEES_NET = "";

                ExportInventoryCSV.WriteDynamicFileRow(record);
            }
            ExportInventoryCSV.Dispose();
        }

        public void ExportAmazonAccountingSalesReport(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/" + CSVFileName + ".csv");
            ExportInventoryCSV.WriteLine("DATE,Type,DETAILS,CURRENCY,GROSS,VAT,NET,vat_rate\n");
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,ORDER_ID,ASIN,GROSS,VAT,NET,vat_rate FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            double Multiplier = 1;
            if (TableName == "Amazon_Orders")
                Multiplier = -1;
            dynamic record = new System.Dynamic.ExpandoObject();
            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                if (!rdr.IsDBNull(0))
                    record.DATE = rdr.GetString(0);
                else
                    record.DATE = "";
                if (!rdr.IsDBNull(1))
                    record.Type = rdr.GetString(1);
                else
                    record.Type = "";
                if (!rdr.IsDBNull(2))
                    record.DETAILS = rdr.GetString(2);
                else
                    record.DETAILS = "";
                record.CURRENCY = "GBP";
                double Gross = rdr.GetDouble(3);
                if (Gross != (double)GlobalVariables.NULLValue)
                    record.GROSS = Math.Round((Gross * Multiplier), 2).ToString();
                else
                    record.GROSS = "";
                double vat = rdr.GetDouble(4);
                if (vat != (double)GlobalVariables.NULLValue)
                    record.VAT = Math.Round((vat * Multiplier), 2).ToString();
                else
                    record.Vat = "";
                double NET = rdr.GetDouble(5);
                if (NET != (double)GlobalVariables.NULLValue)
                    record.NET = Math.Round((NET * Multiplier), 2).ToString();
                else
                    record.NET = "";
                record.FEES = 0;
                double vat_rate = rdr.GetDouble(6);
                if (vat_rate != (double)GlobalVariables.NULLValue)
                    record.VAT_RATE = Math.Round(vat_rate, 2).ToString();
                else
                    record.VAT_RATE = "";

                ExportInventoryCSV.WriteDynamicFileRow(record);
            }

            ExportInventoryCSV.Dispose();
        }

        public void ExportAmazonBlocked(DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/AMAZON_BLOCKED.csv");
            ExportInventoryCSV.WriteLine("date,ORDER_ID,DISPATCH,TITLE,PRICE,vat,BuyerName,Buyeraddr,asin,PAYMENT\n");
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,ORDER_ID,DISPATCH,TITLE,PRICE,vat,BuyerName,Buyeraddr,asin,PAYMENT FROM AMAZON_BLOCKED where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            dynamic record = new System.Dynamic.ExpandoObject();
            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                if (!rdr.IsDBNull(0))
                    record.DATE = rdr.GetString(0);
                else
                    record.DATE = "";
                if (!rdr.IsDBNull(1))
                    record.Id = rdr.GetString(1);
                else
                    record.Id = "";
                if (!rdr.IsDBNull(2))
                    record.DISPATCH = rdr.GetString(2);
                else
                    record.DISPATCH = "";
                if (!rdr.IsDBNull(3))
                    record.TITLE = rdr.GetString(3);
                else
                    record.TITLE = "";
                double Price = rdr.GetDouble(4);
                if (Price != (double)GlobalVariables.NULLValue)
                    record.Price = Math.Round((Price), 2).ToString();
                else
                    record.Price = "";
                double vat = rdr.GetDouble(5);
                if (vat != (double)GlobalVariables.NULLValue)
                    record.VAT = Math.Round(vat, 2).ToString();
                else
                    record.Vat = "";
                if (!rdr.IsDBNull(6))
                    record.Buyer = rdr.GetString(6);
                else
                    record.Buyer = "";
                if (!rdr.IsDBNull(7))
                    record.Address = rdr.GetString(7);
                else
                    record.Address = "";
                if (!rdr.IsDBNull(8))
                    record.ASIN = rdr.GetString(8);
                else
                    record.ASIN = "";
                if (!rdr.IsDBNull(9))
                    record.PaymentMethod = rdr.GetString(9);
                else
                    record.PaymentMethod = "";

                ExportInventoryCSV.WriteDynamicFileRow(record);
            }

            ExportInventoryCSV.Dispose();
        }

        public void ReplaceEbaySale(string date, string name, string address, string transactionId, string itemId, string valGross, double fee, double val_vat, double val_net, double vat_rate)
        {
            int TIDhash = transactionId.GetHashCode();
            long TStamp = DateParser.GetUnixStamp(date);

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE INTO EBAY_SALES (date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fee,feenet,feevat,TransactionID_hash,TStamp) " +
                "VALUES(@date,@BuyerName,@address,@TransactionID,@ItemId,@GROSS,@vat,@net,@vat_rate,@fee,@feenet,@feevat,@TransactionID_hash,@TStamp)";

            cmd.Parameters.AddWithValue("@date", date);
            cmd.Parameters.AddWithValue("@BuyerName", name);
            cmd.Parameters.AddWithValue("@address", address);
            cmd.Parameters.AddWithValue("@TransactionID", transactionId);
            cmd.Parameters.AddWithValue("@ItemId", itemId);
            cmd.Parameters.AddWithValue("@GROSS", valGross);
            cmd.Parameters.AddWithValue("@vat", val_vat);
            cmd.Parameters.AddWithValue("@net", val_net);
            cmd.Parameters.AddWithValue("@vat_rate", vat_rate);
            cmd.Parameters.AddWithValue("@fee", fee);
            double feeNet = Math.Round(fee / 1.2, 2);
            cmd.Parameters.AddWithValue("@feenet", feeNet);
            double feeVat = fee - feeNet;
            cmd.Parameters.AddWithValue("@feevat", feeVat);
            cmd.Parameters.AddWithValue("@TransactionID_hash", TIDhash);
            cmd.Parameters.AddWithValue("@TStamp", TStamp);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }

        public void ReplaceEbayRefund(string OrderType, string date, string name, string address, string transactionId, string itemId, string valGross, double fee, double val_vat, double val_net, double vat_rate)
        {
            int TIDhash = transactionId.GetHashCode();
            long TStamp = DateParser.GetUnixStamp(date);

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE INTO EBAY_REFUNDs (OrderType,date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fee,feenet,feevat,TransactionID_hash,TStamp) " +
                "VALUES(@OrderType,@date,@BuyerName,@address,@TransactionID,@ItemId,@GROSS,@vat,@net,@vat_rate,@fee,@feenet,@feevat,@TransactionID_hash,@TStamp)";

            cmd.Parameters.AddWithValue("@OrderType", OrderType);
            cmd.Parameters.AddWithValue("@date", date);
            cmd.Parameters.AddWithValue("@BuyerName", name);
            cmd.Parameters.AddWithValue("@address", address);
            cmd.Parameters.AddWithValue("@TransactionID", transactionId);
            cmd.Parameters.AddWithValue("@ItemId", itemId);
            cmd.Parameters.AddWithValue("@GROSS", valGross);
            cmd.Parameters.AddWithValue("@vat", val_vat);
            cmd.Parameters.AddWithValue("@net", val_net);
            cmd.Parameters.AddWithValue("@vat_rate", vat_rate);
            cmd.Parameters.AddWithValue("@fee", fee);
            double feeNet = Math.Round(fee / 1.2, 2);
            cmd.Parameters.AddWithValue("@feenet", feeNet);
            double feeVat = fee - feeNet;
            cmd.Parameters.AddWithValue("@feevat", feeVat);
            cmd.Parameters.AddWithValue("@TransactionID_hash", TIDhash);
            cmd.Parameters.AddWithValue("@TStamp", TStamp);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }

        public string EbayGetItemID(string transactionId)
        {
            int TIDhash = transactionId.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ItemId FROM EBAY_SALES where TransactionID_hash=@TIDhash and TransactionID=@TID";
            cmd.Parameters.AddWithValue("@TIDhash", TIDhash);
            cmd.Parameters.AddWithValue("@TID", transactionId);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                //check if the existing row has values
                if (!rdr.IsDBNull(0))
                    return rdr.GetString(0);
            }

            return "";
        }

        public bool EbayCheckRefundExists(string date, string type, string transactionId, string GROSS)
        {
            int TIDhash = transactionId.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ItemId FROM EBAY_REFUNDS where TransactionID_hash=@TIDhash and TransactionID=@TID and GROSS=@GROSS and date=@date and ordertype=@type";
            cmd.Parameters.AddWithValue("@TIDhash", TIDhash);
            cmd.Parameters.AddWithValue("@TID", transactionId);
            cmd.Parameters.AddWithValue("@GROSS", GROSS);
            cmd.Parameters.AddWithValue("@date", date);
            cmd.Parameters.AddWithValue("@type", type);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                //check if the existing row has values
                if (!rdr.IsDBNull(0))
                {
                    string ItemId = rdr.GetString(0);
                    if (ItemId.Length > 0)
                        return true;
                }
            }

            return false;
        }

        public void ExportEbaySalesRefunds(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            string ExtraCol = "";
            if (TableName == "EBAY_REFUNDS")
                ExtraCol = ",OrderType";
            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/" + CSVFileName + ".csv");
            ExportInventoryCSV.WriteLine("Transaction date,Order number,Buyer username,Post to postcode,Item ID,Net amount,VAT,VAT_NET,VAT_RATE,FEE,FEE_NET,FEE_VAT" + ExtraCol + "\n");
            dynamic record = new System.Dynamic.ExpandoObject();
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,TransactionID,BuyerName,address,ItemId,GROSS,vat,net,vat_rate,fee,feenet,feevat" + ExtraCol + " FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                record.date = GetNonNullString(rdr, 0);
                record.ORDER_ID = GetNonNullString(rdr, 1);
                record.BuyerName = GetNonNullString(rdr, 2);
                record.address = GetNonNullString(rdr, 3);
                record.ItemId = GetNonNullString(rdr, 4);
                record.GROSS = GetNonNullDouble(rdr, 5);
                record.vat = GetNonNullDouble(rdr, 6);
                record.vatNet = GetNonNullDouble(rdr, 7);
                record.vatRate = GetNonNullDouble(rdr, 8);
                record.fee = GetNonNullDouble(rdr, 9);
                record.feeNet = GetNonNullDouble(rdr, 10);
                record.feeVat = GetNonNullDouble(rdr, 11);
                if (double.Parse(record.fee) != double.Parse(record.feeVat) + double.Parse(record.feeNet))
                    record.feeVat = (FloatSubstract(double.Parse(record.fee), double.Parse(record.feeNet), 3)).ToString();
                if (ExtraCol.Length > 0)
                    record.type = GetNonNullString(rdr, 12);

                ExportInventoryCSV.WriteDynamicFileRow(record);
            }
            ExportInventoryCSV.Dispose();
        }

        public void ExportEbayAccountingSalesReport(string TableName, string CSVFileName, DateTime StartDate, DateTime EndDate)
        {
            long TStart = DateParser.DateTimeToMinutes(StartDate);
            long TEnd = DateParser.DateTimeToMinutes(EndDate);

            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateDynamicFile("./reports/" + CSVFileName + ".csv");
            ExportInventoryCSV.WriteLine("DATE,TYPE,DETAILS,CURRENCY,GROSS,VAT,NET,VAT RATE,FEES_GROSS,FEES_VAT,FEES_NET\n");
            dynamic record = new System.Dynamic.ExpandoObject();
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,TransactionID,ItemId,GROSS,vat,net,vat_rate,fee,feevat,feenet FROM " + TableName + " where TStamp >= @TStart and TStamp <= @TEnd order by TStamp asc";
            cmd.Parameters.AddWithValue("@TStart", TStart);
            cmd.Parameters.AddWithValue("@TEnd", TEnd);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                record.DATE = GetNonNullString(rdr, 0);
                record.Type = GetNonNullString(rdr, 1);
                record.DETAILS = GetNonNullString(rdr, 2);
                record.CURRENCY = "GBP";
                record.GROSS = GetNonNullDouble(rdr, 3);
                record.Vat = GetNonNullDouble(rdr, 4);
                record.NET = GetNonNullDouble(rdr, 5);
                record.VAT_RATE = GetNonNullDouble(rdr, 6);
                record.Fee = GetNonNullDouble(rdr, 7);
                record.FEES_VAT = GetNonNullDouble(rdr, 8);
                record.FEES_NET = GetNonNullDouble(rdr, 9);
                if (double.Parse(record.Fee) != double.Parse(record.FEES_VAT) + double.Parse(record.FEES_NET))
                    record.FEES_VAT = (FloatSubstract(double.Parse(record.Fee),double.Parse(record.FEES_NET),3)).ToString();

                ExportInventoryCSV.WriteDynamicFileRow(record);

            }
            ExportInventoryCSV.Dispose();
        }

        string GetNonNullDouble(SQLiteDataReader rdr, int col)
        {
            double val = rdr.GetDouble(col);
            if (val != (double)GlobalVariables.NULLValue)
                return Math.Round(val, 2).ToString();
            return "";
        }

        string GetNonNullString(SQLiteDataReader rdr, int col)
        {
            if (!rdr.IsDBNull(col))
                return rdr.GetString(col);
            return "";
        }

        public void UpdateEbayVAT()
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            /*
                        var cmd3 = new SQLiteCommand(m_dbConnection);
                        cmd3.CommandText = "delete from InventoryCSV where ebay_id_str=@ebay_id_str";
                        cmd3.Parameters.AddWithValue("@ebay_id_str", "324191194998");
                        cmd3.Prepare();
                        cmd3.ExecuteNonQuery();

                        ReplaceInventoryRow("324191194998", "1", "2");
                        ReplaceInventoryRow("324275307804", "2", "3");
                        ReplaceInventoryRow("324267599466", "3", "4");
                        ReplaceInventoryRow("324225064311", "4", "5");
                        ReplaceInventoryRow("324099096923", "5", "6");
                        ReplaceInventoryRow("324270711679", "6", "7");
                        ReplaceInventoryRow("323958566143", "7", "8"); */

            GlobalVariables.Logger.Log("Update Ebay VAT - Started");
            GlobalVariables.ImportingToDBBlock = "UpdateEbayVat";
            WriteCSVFile ExportSalesCSV = new WriteCSVFile();
            ExportSalesCSV.CreateDynamicFile("./reports/no-vat-ebay-sales.csv");
            ExportSalesCSV.WriteLine("date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fees,feesnet,feesvat\n");
            WriteCSVFile ExportSalesRunCSV = new WriteCSVFile();
            ExportSalesRunCSV.CreateDynamicFile("./reports/ebay-sales-vat.csv");
            ExportSalesRunCSV.WriteLine("date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fees,feesnet,feesvat\n");
            WriteCSVFile ExportRefundsCSV = new WriteCSVFile();
            ExportRefundsCSV.CreateDynamicFile("./reports/no-vat-ebay-refunds.csv");
            ExportRefundsCSV.WriteLine("date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fees,feesnet,feesvat,ordertype\n");
            WriteCSVFile ExportRefundsRunCSV = new WriteCSVFile();
            ExportRefundsRunCSV.CreateDynamicFile("./reports/ebay-refunds-vat-run.csv");
            ExportRefundsRunCSV.WriteLine("date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fees,feesnet,feesvat,ordertype\n");
            for (int i = 0; i < 2; i++)
            {
                var cmd = new SQLiteCommand(m_dbConnection);
                if (i == 0)
                    cmd.CommandText = "SELECT date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fee,feenet,feevat FROM ebay_sales where vat_rate=" + GlobalVariables.NULLValue.ToString() + "";
                else
                    cmd.CommandText = "SELECT date,BuyerName,address,TransactionID,ItemId,GROSS,vat,net,vat_rate,fee,feenet,feevat,ordertype FROM ebay_refunds where vat_rate=" + GlobalVariables.NULLValue.ToString() + "";
                cmd.Prepare();

                dynamic record = new System.Dynamic.ExpandoObject();
                SQLiteDataReader rdr = cmd.ExecuteReader();
                while (rdr.Read() && rdr.HasRows == true)
                {
                    //get the itemID
                    string EbayId = GetNonNullString(rdr, 4);
                    double InventoryVAT = GlobalVariables.NULLValue;

                    //check if inventory has this value
                    if (EbayId.Length > 0)
                    {
                        int ebayhash = EbayId.GetHashCode();
                        var cmd1 = new SQLiteCommand(m_dbConnection);
                        cmd1.CommandText = "SELECT vat FROM InventoryCSV where ebay_id_Hash=@ebay_id_Hash and ebay_id_str=@ebay_id_str limit 1";
                        cmd1.Parameters.AddWithValue("@ebay_id_Hash", ebayhash);
                        cmd1.Parameters.AddWithValue("@ebay_id_str", EbayId);
                        cmd1.Prepare();
                        SQLiteDataReader rdr2 = cmd1.ExecuteReader();

                        if (rdr2.Read() && rdr2.HasRows == true)
                            InventoryVAT = rdr2.GetDouble(0);
                    }

                    record.Date = GetNonNullString(rdr, 0);
                    record.Name = GetNonNullString(rdr, 1);
                    record.address = GetNonNullString(rdr, 2);
                    record.TID = GetNonNullString(rdr, 3);
                    record.IID = GetNonNullString(rdr, 4);
                    record.Gross = GetNonNullDouble(rdr, 5);
                    record.vat = GetNonNullDouble(rdr, 6);
                    record.vatnet = GetNonNullDouble(rdr, 7);
                    record.vat_rate = GetNonNullDouble(rdr, 8);
                    record.Fee = GetNonNullDouble(rdr, 9);
                    record.FeeNet = GetNonNullDouble(rdr, 10);
                    record.FeeVat = GetNonNullDouble(rdr, 11);
                    if (i == 1)
                        record.ordertype = GetNonNullString(rdr, 12);

                    if (InventoryVAT == GlobalVariables.NULLValue)
                    {
                        //write it to the csv
                        if (i == 0)
                            ExportSalesCSV.WriteDynamicFileRow(record);
                        else
                            ExportRefundsCSV.WriteDynamicFileRow(record);
                    }
                    //we can update vat if we got here
                    else
                    {
                        string Transaction_ID = GetNonNullString(rdr, 3);
                        int Transaction_ID_Hash = Transaction_ID.GetHashCode();

                        double Gross = rdr.GetDouble(5);
                        double NET = Gross / (1 + InventoryVAT / 100);
                        NET = Math.Round(NET, 2);
                        double vat = FloatSubstract(Gross, NET, 3);

                        var cmd2 = new SQLiteCommand(m_dbConnection);
                        if (i == 0)
                            cmd2.CommandText = "UPDATE ebay_sales set vat_rate=@vat_rate,net=@net,vat=@vat where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol";
                        else
                        {
                            cmd2.CommandText = "UPDATE ebay_refunds set vat_rate=@vat_rate,net=@net,vat=@vat where TransactionID_hash=@IdColhash and TransactionID=@TransactionIDCol and date=@date and gross=@gross and ordertype=@ordertype";
                            cmd2.Parameters.AddWithValue("@date", record.Date);
                            cmd2.Parameters.AddWithValue("@gross", Gross);
                            cmd2.Parameters.AddWithValue("@ordertype", record.ordertype);
                        }
                        cmd2.Parameters.AddWithValue("@vat_rate", InventoryVAT);
                        cmd2.Parameters.AddWithValue("@net", NET);
                        cmd2.Parameters.AddWithValue("@vat", vat);
                        cmd2.Parameters.AddWithValue("@IdColhash", Transaction_ID_Hash);
                        cmd2.Parameters.AddWithValue("@TransactionIDCol", Transaction_ID);
                        cmd2.Prepare();
                        cmd2.ExecuteNonQuery();

                        record.vat = vat;
                        record.vatnet = NET;
                        record.vat_rate = InventoryVAT;

                        //write it to the csv
                        if (i == 0)
                            ExportSalesRunCSV.WriteDynamicFileRow(record);
                        else
                            ExportRefundsRunCSV.WriteDynamicFileRow(record);
                    }
                }
            }
            GlobalVariables.ImportingToDBBlock = "";

            ExportSalesCSV.Dispose();
            ExportRefundsCSV.Dispose();
            ExportSalesRunCSV.Dispose();
            ExportRefundsRunCSV.Dispose();
            GlobalVariables.Logger.Log("Update Ebay VAT - Finished");
        }
        public void UpdateV1DB()
        {
            GlobalVariables.Logger.Log("Started upgrading InventoryCSV");
            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ebay_id_Hash,ebay_id_str,asin_str,asin_hash FROM InventoryCSV";
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                int ebay_id_hash = rdr.GetInt32(0);
                string ebay_id_str = GetNonNullString(rdr,1);
                int new_ebay_Hash = ebay_id_str.GetHashCode();

                string asin_str = GetNonNullString(rdr, 2);
                int asin_hash = 0;
                if (!rdr.IsDBNull(3))
                    asin_hash = rdr.GetInt32(3);
                int new_asin_Hash = asin_str.GetHashCode();

                if (ebay_id_hash != new_ebay_Hash || asin_hash != new_asin_Hash)
                {
                    var cmd2 = new SQLiteCommand(m_dbConnection);
                    cmd2.CommandText = "update InventoryCSV set ebay_id_hash=@ebay_id_hash_new,asin_hash=@new_asin_hash where ebay_id_hash=@ebay_id_hash and ebay_id_str=@ebay_id_str and asin_str=@asin_str and asin_hash=@asin_hash";
                    cmd2.Parameters.AddWithValue("@ebay_id_hash_new", new_ebay_Hash);
                    cmd2.Parameters.AddWithValue("@ebay_id_hash", ebay_id_hash);
                    cmd2.Parameters.AddWithValue("@ebay_id_str", ebay_id_str);
                    cmd2.Parameters.AddWithValue("@asin_str", asin_str);
                    cmd2.Parameters.AddWithValue("@asin_hash", asin_hash);
                    cmd2.Parameters.AddWithValue("@new_asin_hash", new_asin_Hash);
                    cmd2.Prepare();
                    int AffectedCount = cmd2.ExecuteNonQuery();
                    Debug.Assert(AffectedCount == 1);
                }
            }
            GlobalVariables.Logger.Log("Finished upgrading InventoryCSV");

            string[] AmazonTables = { "AMAZON_ORDERS", "AMAZON_REFUNDS", "AMAZON_BLOCKED" };
            foreach (string Table in AmazonTables)
            {
                GlobalVariables.Logger.Log("Started upgrading " + Table);
                cmd = new SQLiteCommand(m_dbConnection);
                cmd.CommandText = "SELECT asin_hash,ORDER_ID_hash,asin,ORDER_ID FROM "+ Table;
                cmd.Prepare();

                rdr = cmd.ExecuteReader();
                while (rdr.Read() && rdr.HasRows == true)
                {
                    int asin_hash = rdr.GetInt32(0);
                    int ORDER_ID_hash = rdr.GetInt32(1);
                    string asin = GetNonNullString(rdr, 2);
                    string ORDER_ID = GetNonNullString(rdr, 3);
                    int new_asin_hash = asin.GetHashCode();
                    int new_ORDER_ID_hash = ORDER_ID.GetHashCode();

                    if (asin_hash != new_asin_hash || ORDER_ID_hash != new_ORDER_ID_hash)
                    {
                        var cmd2 = new SQLiteCommand(m_dbConnection);
                        cmd2.CommandText = "update " + Table + " set asin_hash=@asin_hash_new,ORDER_ID_hash=@ORDER_ID_hash_new where asin_hash=@asin_hash and ORDER_ID_hash=@ORDER_ID_hash and ORDER_ID=@ORDER_ID and asin=@asin";
                        cmd2.Parameters.AddWithValue("@ORDER_ID_hash_new", new_ORDER_ID_hash);
                        cmd2.Parameters.AddWithValue("@asin_hash_new", new_asin_hash);
                        cmd2.Parameters.AddWithValue("@asin_hash", asin_hash);
                        cmd2.Parameters.AddWithValue("@ORDER_ID_hash", ORDER_ID_hash);
                        cmd2.Parameters.AddWithValue("@ORDER_ID", ORDER_ID);
                        cmd2.Parameters.AddWithValue("@asin", asin);
                        cmd2.Prepare();
                        int AffectedCount = cmd2.ExecuteNonQuery();
                        Debug.Assert(AffectedCount == 1);
                    }
                }
                GlobalVariables.Logger.Log("Finished upgrading " + Table);
            }

            string[] PaypalTables = { "PAYPAL_SALES", "PAYPAL_REFUNDS", "EBAY_SALES", "EBAY_REFUNDS" };
            foreach (string Table in PaypalTables)
            {
                GlobalVariables.Logger.Log("Started upgrading " + Table);
                cmd = new SQLiteCommand(m_dbConnection);
                cmd.CommandText = "SELECT TransactionID_hash,TransactionID FROM " + Table;
                cmd.Prepare();

                rdr = cmd.ExecuteReader();
                while (rdr.Read() && rdr.HasRows == true)
                {
                    int TransactionID_hash = rdr.GetInt32(0);
                    string TransactionID = GetNonNullString(rdr, 1);
                    int TransactionID_hash_new = TransactionID.GetHashCode();

                    if (TransactionID_hash_new != TransactionID_hash)
                    {
                        var cmd2 = new SQLiteCommand(m_dbConnection);
                        cmd2.CommandText = "update " + Table + " set TransactionID_hash=@TransactionID_hash_new where TransactionID=@TransactionID and TransactionID_hash=@TransactionID_hash";
                        cmd2.Parameters.AddWithValue("@TransactionID_hash_new", TransactionID_hash_new);
                        cmd2.Parameters.AddWithValue("@TransactionID", TransactionID);
                        cmd2.Parameters.AddWithValue("@TransactionID_hash", TransactionID_hash);
                        cmd2.Prepare();
                        int AffectedCount = cmd2.ExecuteNonQuery();
                        Debug.Assert(AffectedCount == 1);
                    }
                }
                GlobalVariables.Logger.Log("Finished upgrading " + Table);
            }

            GlobalVariables.Logger.Log("Finished upgrading db");
        }
    }
}
