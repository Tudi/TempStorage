using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SQLite;

namespace CSVIngester
{
    public enum CachedIndexTypes
    {
        CIT_PaypalSales = 1,
        CIT_COUNT
    }
    public class IndexCache
    {
        public Dictionary<int, List<string>> CachedIndexes = new Dictionary<int, List<string>>();
        public void CacheExistingIndexes(CachedIndexTypes cit)
        {
            if (cit == CachedIndexTypes.CIT_PaypalSales)
            {
                CachedIndexes.Clear();

                SQLiteConnection m_dbConnection = GlobalVariables.DBStorage.GetConnection();
                var cmd1 = new SQLiteCommand(m_dbConnection);
                cmd1.CommandText = "SELECT TransactionID_hash,TransactionID FROM PAYPAL_SALES";
                cmd1.Prepare();

                SQLiteDataReader rdr = cmd1.ExecuteReader();
                while (rdr.Read() && rdr.HasRows == true)
                {
                    if (!rdr.IsDBNull(1))
                    {
                        int CurHash = rdr.GetInt32(0);
                        List<string> Temp;
                        CachedIndexes.TryGetValue(CurHash, out Temp);
                        string NewIndex = rdr.GetString(1);
                        if (Temp != null && Temp.Contains(NewIndex))
                            continue;
                        AddRow(CurHash, NewIndex);
                    }
                }
            }
        }
        public bool CheckRowExists(int Hash, string Index)
        {
            List<string> Temp;
            if (CachedIndexes.TryGetValue(Hash, out Temp) == false)
                return false;
            if (Temp.Contains(Index))
                return true;
            return false;
        }
        public void AddRow(int Hash, string Index)
        {
            List<string> l;
            if (CachedIndexes.TryGetValue(Hash, out l) == false)
            {
                l = new List<string>();
                l.Add(Index);
                CachedIndexes.Add(Hash,l);
            }
            else
            {
                if (l.Contains(Index) == false)
                    l.Add(Index);
            }
        }
    }
}
