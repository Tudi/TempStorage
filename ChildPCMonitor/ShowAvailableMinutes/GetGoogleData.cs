using Google.Apis.Auth.OAuth2;
using Google.Apis.Discovery.v1;
using Google.Apis.Services;
using Google.Apis.Sheets.v4;
using Google.Apis.Sheets.v4.Data;
using Google.Apis.Util.Store;
using Microsoft.Extensions.Configuration;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Threading;
namespace ShowAvailableMinutes
{
    internal class GetGoogleData
    {
        private SheetsService googleService;
        private string spreadsheetId;
        private string sheetName;

        public GetGoogleData()
        {
            // Load configuration
            var configuration = new ConfigurationBuilder()
                .AddJsonFile("appsettings.json", optional: false, reloadOnChange: true)
                .Build();

            var clientId = configuration["Google:ClientId"];
            var clientSecret = configuration["Google:ClientSecret"];
            spreadsheetId = new string(configuration["Google:SpreadsheetId"]);
            sheetName = new string(configuration["Google:SheetName"]);

            UserCredential credential;

            using (var stream = new FileStream("token.json", FileMode.OpenOrCreate))
            {
                credential = GoogleWebAuthorizationBroker.AuthorizeAsync(
                    new ClientSecrets
                    {
                        ClientId = clientId,
                        ClientSecret = clientSecret
                    },
                    new[] { SheetsService.Scope.SpreadsheetsReadonly },
                    "user",
                    CancellationToken.None,
                    new FileDataStore("StoredCredential")).Result;
            }

            // Create Google Sheets API service
            googleService = new SheetsService(new Google.Apis.Services.BaseClientService.Initializer()
            {
                HttpClientInitializer = credential,
                ApplicationName = "Google Sheets API C# Console App",
            });
        }

        public int GetMinutesAvailable()
        {

            // Define request parameters
            var range = $"{sheetName}!A:B";
            SpreadsheetsResource.ValuesResource.GetRequest request =
                    googleService.Spreadsheets.Values.Get(spreadsheetId, range);

            // Retrieve data from spreadsheet
            ValueRange response = request.Execute();
            IList<IList<object>> values = response.Values;

            double ret = 15;
            if (values != null && values.Count > 0)
            {
                foreach (var row in values)
                {
                    if (row.Count >= 1)
                    {
                        if (row[0] != null && row[0].ToString() != null)
                        {
                            ret += Double.Parse(row[0].ToString()) * 60;
                        }
                    }
                }
            }
            return (int)ret;
        }
    }
}
