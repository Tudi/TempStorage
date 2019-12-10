using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SQLite;
using System.Transactions;

namespace CSVIngester
{
    public class DBHandler
    {
        public SQLiteConnection m_dbConnection = null;
        public DBHandler()
        {
//            SQLiteConnection.CreateFile("EbayCSV.db");
            m_dbConnection = new SQLiteConnection("Data Source=EbayCSV.db;New=False;Version=3;journal_mode=WAL;synchronous=NORMAL");
            m_dbConnection.Open();

            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "PRAGMA cache_size=100000";
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

                    string sql = "create table InventoryCSV (ebay_id_Hash INT4,asin_hash INT4,vat FLOAT DEFAULT -1, ebay_id_str varchar(20),asin_str varchar(20))";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE UNIQUE INDEX IF NOT EXISTS UniqueItemId ON InventoryCSV (ebay_id_Hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON InventoryCSV (asin_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }

            using (TransactionScope tran = new TransactionScope())
            {
                SQLiteCommand command = m_dbConnection.CreateCommand();
                command.CommandText = "SELECT name FROM sqlite_master WHERE name='AMAZON_ORDERS'";
                var name = command.ExecuteScalar();

                if (!(name != null && name.ToString() == "AMAZON_ORDERS"))
                {

                    string sql = "create table AMAZON_ORDERS (date varchar(20),ORDER_ID varchar(20),TITLE varchar(200),GROSS float,vat float,BuyerName varchar(200),Buyeraddr varchar(200), asin varchar(20), NET float, vat_rate float, asin_hash int4, ORDER_ID_hash int4)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE UNIQUE INDEX IF NOT EXISTS UniqueOrderId ON AMAZON_ORDERS (ORDER_ID)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();

                    sql = "CREATE INDEX IF NOT EXISTS UniqueProductId ON AMAZON_ORDERS (asin_hash)";
                    command = new SQLiteCommand(sql, m_dbConnection);
                    command.ExecuteNonQuery();
                }
            }
        }

        private void ReplaceInventoryRow(string ebay_id, string asin, string vat)
        {
            int ebayhash = ebay_id.GetHashCode();
            int asinhash = asin.GetHashCode();

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE INTO InventoryCSV(ebay_id_Hash,asin_hash,vat,ebay_id_str,asin_str) VALUES(@ebay_id_Hash, @asin_hash, @vat,@ebay_id_str,@asin_str)";

            cmd.Parameters.AddWithValue("@ebay_id_Hash", ebayhash);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@vat", float.Parse(vat));
            cmd.Parameters.AddWithValue("@ebay_id_str", ebay_id); 
            cmd.Parameters.AddWithValue("@asin_str", asin);
            cmd.Prepare();

            cmd.ExecuteNonQuery(); 
        }
        public enum InvenotryInsertResultCodes
        {
            RowExistedButWasEmpty = 1,
            RowExisted = 2,
            RowDidNotExistInsertedNew = 3,
            RowInvalidValues = 4,
        }
        public InvenotryInsertResultCodes InsertInventory(string ebay_id, string asin, string vat)
        {
            if (ebay_id == null || ebay_id.Length == 0 || asin == null || asin.Length == 0)
                return InvenotryInsertResultCodes.RowInvalidValues;

            InvenotryInsertResultCodes ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            int ebayhash = ebay_id.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ebay_id_Hash,asin_str,vat FROM InventoryCSV where ebay_id_Hash=@ebayhash and ebay_id_str=@ebay_id";
            cmd.Parameters.AddWithValue("@ebayhash", ebayhash);
            cmd.Parameters.AddWithValue("@ebay_id", ebay_id);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if(rdr.Read() && rdr.HasRows == true)
            {
                //check if the existing row has values
                string asin_str = "";
                if (!rdr.IsDBNull(1))
                    asin_str = rdr.GetString(1);
                if (asin_str == null || asin_str.Length == 0)
                {
                    //value exists, but it still needs import
                    ReplaceInventoryRow(ebay_id, asin, vat);
                    ReturnCode = InvenotryInsertResultCodes.RowExistedButWasEmpty;
                }
                else
                {
                    ReturnCode = InvenotryInsertResultCodes.RowExisted;
                }
            }
            else
            {
                ReplaceInventoryRow(ebay_id, asin, vat);
                ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;/**/
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
            if(m_dbConnection != null)
                m_dbConnection = null;
        }

        public void UpdateInventoryVatEbay(string ebay_id, string vat)
        {
            if (ebay_id == null || ebay_id.Length == 0 || vat == null || vat.Length == 0)
                return;

            int ebayhash = ebay_id.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE into InventoryCSV(ebay_id_Hash,vat,ebay_id_str)values(@ebayhash,@vat,@ebay_id)";
            cmd.Parameters.AddWithValue("@ebayhash", ebayhash);
            cmd.Parameters.AddWithValue("@vat", float.Parse(vat));
            cmd.Parameters.AddWithValue("@ebay_id", ebay_id);
            cmd.Prepare();
            cmd.ExecuteNonQuery();
        }

        public void UpdateInventoryVatAsin(string asin, string vat)
        {
            if (asin == null || asin.Length == 0 || vat == null || vat.Length == 0)
                return;

            int asinHash = asin.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE into InventoryCSV(asin_hash,vat,asin_str)values(@asin_hash,@vat,@asin_str)";
            cmd.Parameters.AddWithValue("@asin_hash", asinHash);
            cmd.Parameters.AddWithValue("@vat", float.Parse(vat));
            cmd.Parameters.AddWithValue("@asin_str", asin);
            cmd.Prepare();
            cmd.ExecuteNonQuery();
        }

        public void ExportInventoryTable()
        {
            System.IO.Directory.CreateDirectory("./reports");
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
                float vat = rdr.GetFloat(2);
                ExportInventoryCSV.InventoryRunFileAddRow(ebay_id_str, asin_str, vat);
            }
            ExportInventoryCSV.Dispose();
        }

        private void InsertAmazonRow(string DateCol, string IdCol, string TitleCol, string PriceCol, string VATCol, string BuyerCol, string AddressCol, string ASINCol, float NET, float VAT_RATE)
        {
            int IdColhash = IdCol.GetHashCode();
            int asinhash = ASINCol.GetHashCode();

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "REPLACE INTO AMAZON_ORDERS(date,ORDER_ID,TITLE,GROSS,vat,BuyerName,Buyeraddr,asin,NET,vat_rate,asin_hash,ORDER_ID_hash) " +
                "VALUES(@date,@ORDER_ID,@TITLE,@GROSS,@vat,@BuyerName,@Buyeraddr,@asin,@NET,@vat_rate,@asin_hash,@ORDER_ID_hash)";

            cmd.Parameters.AddWithValue("@date", DateCol);
            cmd.Parameters.AddWithValue("@ORDER_ID", IdCol);
            cmd.Parameters.AddWithValue("@TITLE", TitleCol);
            cmd.Parameters.AddWithValue("@GROSS", PriceCol);
            cmd.Parameters.AddWithValue("@vat", float.Parse(VATCol));
            cmd.Parameters.AddWithValue("@BuyerName", BuyerCol);
            cmd.Parameters.AddWithValue("@Buyeraddr", AddressCol);
            cmd.Parameters.AddWithValue("@asin", ASINCol);
            cmd.Parameters.AddWithValue("@NET", NET);
            cmd.Parameters.AddWithValue("@vat_rate", VAT_RATE);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@ORDER_ID_hash", IdColhash);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }

        public void UpdateAllMissingInventoryRows(string ASINCol, string VATCol)
        {
            int asinhash = ASINCol.GetHashCode();
            float VAT = float.Parse(VATCol);

            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "update InventoryCSV set vat=@VAT where asin_hash=@asin_hash and vat<0 and asin_str=@asin";

            cmd.Parameters.AddWithValue("@VAT", VAT);
            cmd.Parameters.AddWithValue("@asin_hash", asinhash);
            cmd.Parameters.AddWithValue("@asin", ASINCol);
            cmd.Prepare();

            cmd.ExecuteNonQuery();
        }
        public InvenotryInsertResultCodes InsertAmazonOrder(string DateCol, string IdCol, string TitleCol, string PriceCol, string VATCol, string BuyerCol, string AddressCol, string ASINCol, float NET, float VAT_RATE)
        {
            InvenotryInsertResultCodes ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
            int ORDER_ID_hashhash = IdCol.GetHashCode();

            //check if ts record already exists
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT ORDER_ID_hash FROM AMAZON_ORDERS where ORDER_ID_hash=@ORDER_ID_hash";
            cmd.Parameters.AddWithValue("@ORDER_ID_hash", ORDER_ID_hashhash);
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            if (rdr.Read() && rdr.HasRows == true)
            {
                ReturnCode = InvenotryInsertResultCodes.RowExisted;
            }
            else
            {
                InsertAmazonRow(DateCol, IdCol, TitleCol, PriceCol, VATCol, BuyerCol, AddressCol, ASINCol, NET, VAT_RATE);
                UpdateAllMissingInventoryRows(ASINCol, VATCol);
                ReturnCode = InvenotryInsertResultCodes.RowDidNotExistInsertedNew;
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
        public void ExportAmazonOrdersTable()
        {
            System.IO.Directory.CreateDirectory("./reports");
            WriteCSVFile ExportInventoryCSV = new WriteCSVFile();
            ExportInventoryCSV.CreateAmazonOrdersFile("./reports/AMAZON-ORDERS.csv");
            var cmd = new SQLiteCommand(m_dbConnection);
            cmd.CommandText = "SELECT date,ORDER_ID,TITLE,GROSS,vat,BuyerName,Buyeraddr,asin,NET,vat_rate FROM AMAZON_ORDERS";
            cmd.Prepare();

            SQLiteDataReader rdr = cmd.ExecuteReader();
            while (rdr.Read() && rdr.HasRows == true)
            {
                string datecol = "";
                if (!rdr.IsDBNull(0))
                    datecol = rdr.GetString(0);
                string ORDER_ID = "";
                if (!rdr.IsDBNull(1))
                    ORDER_ID = rdr.GetString(1);
                string TITLE = "";
                if (!rdr.IsDBNull(2))
                    TITLE = rdr.GetString(2);
                float GROSS = rdr.GetFloat(3);
                float vat = rdr.GetFloat(4);
                string BuyerName = "";
                if (!rdr.IsDBNull(5))
                    BuyerName = rdr.GetString(5);
                string Buyeraddr = "";
                if (!rdr.IsDBNull(6))
                    Buyeraddr = rdr.GetString(6);
                string asin = "";
                if (!rdr.IsDBNull(7))
                    asin = rdr.GetString(7);
                float NET = rdr.GetFloat(8);
                float vat_rate = rdr.GetFloat(9);
                ExportInventoryCSV.AmazonOrdersExportFileAddRow(datecol, ORDER_ID, TITLE, GROSS, vat, BuyerName, Buyeraddr, asin, NET, vat_rate);
            }
            ExportInventoryCSV.Dispose();
        }
    }
}
