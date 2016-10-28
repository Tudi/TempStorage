using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using System.Xml.Linq;
using MahApps.Metro.Controls;
using Microsoft.Win32;
using Ookii.Dialogs.Wpf;
using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Series;
using OxyPlot.Wpf;
using CategoryAxis = OxyPlot.Axes.CategoryAxis;
using ColumnSeries = OxyPlot.Series.ColumnSeries;
using LinearAxis = OxyPlot.Axes.LinearAxis;
using LineSeries = OxyPlot.Series.LineSeries;
using SvgExporter = OxyPlot.Wpf.SvgExporter;

namespace JazzBurnReport
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : MetroWindow
    {
        readonly HttpClient client = new HttpClient(new HttpClientHandler { AllowAutoRedirect = false });
        bool blocked;

        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += (sender, args) => BlockInteraction(Render());
        }

        async void BlockInteraction(Task t)
        {
            blocked = true;
            CommandManager.InvalidateRequerySuggested();
            message.Text = string.Empty;
            overlay.Visibility = Visibility.Visible;
            try
            {
                await t;
                overlay.Visibility = Visibility.Hidden;
            }
            catch (ApplicationException ex)
            {
                message.Text = ex.Message;
            }
            finally
            {
                blocked = false;
                CommandManager.InvalidateRequerySuggested();
            }
        }

        async Task<XDocument> Get(Uri uri)
        {
            HttpStatusCode statusCode;
            using (var result = await client.GetAsync(uri))
            {
                if (result.IsSuccessStatusCode)
                {
                    using (var stream = await result.Content.ReadAsStreamAsync())
                    {
                        return await Task.Run(() => XDocument.Load(stream));
                    }
                }
                statusCode = result.StatusCode;
                IEnumerable<string> authHeader;
                if (statusCode == HttpStatusCode.Found &&
                    result.Headers.TryGetValues("X-com-ibm-team-repository-web-auth-msg", out authHeader) &&
                    authHeader.SequenceEqual(new[] {"authrequired"}))
                {
                    statusCode = HttpStatusCode.Unauthorized;
                }
            }
            if (statusCode == HttpStatusCode.Unauthorized)
            {
                var dialog = new CredentialDialog
                {
                    MainInstruction = "Enter your Jazz credentials",
                    Content = "This is probably the same as your corp credentials.",
                    WindowTitle = this.Title,
                    Target = "jazz:clm1.waters.com"
                };
                if (dialog.ShowDialog(this) != true)
                {
                    throw new ApplicationException("User did not enter credentials");
                }
                var authUri = new Uri("https://clm1.waters.com/jts/authenticated/j_security_check");
                //var request = new HttpRequestMessage(HttpMethod.Get, authUri);
                var authBody = new FormUrlEncodedContent(new Dictionary<string, string>
                {
                    { "j_username", dialog.UserName },
                    { "j_password", dialog.Password }
                });
                using (var authResult = await client.PostAsync(authUri, authBody))
                {
                    if (authResult.IsSuccessStatusCode && authResult.StatusCode != HttpStatusCode.Found ||
                        authResult.Headers.Location.AbsolutePath != "/jts/authenticated/")
                    {
                        throw new ApplicationException("Authentication failed");
                    }
                }
                return await Get(uri);
            }
            throw new ApplicationException($"Got error {statusCode}");
        }

        DateTimeOffset ParseDateTimeOffset(XElement element)
        {
            return DateTimeOffset.ParseExact((string) element, "yyyy-MM-ddTHH:mm:ss.fffzzz", CultureInfo.InvariantCulture);
        }

        SprintData LoadData()
        {
            try
            {
                using (var fs = File.OpenRead("data.xml"))
                {
                    return (SprintData)new DataContractSerializer(typeof (SprintData)).ReadObject(fs);
                }
            }
            catch (FileNotFoundException)
            {
                return new SprintData();
            }
        }

        void SaveData(SprintData data)
        {
            using (var fs = File.OpenWrite("data.xml"))
            {
                new DataContractSerializer(typeof(SprintData)).WriteObject(fs, data);
                fs.SetLength(fs.Position);
            }
        }

        async Task Render()
        {
            var data = await Task.Run(() => LoadData());
            Render(data);
        }

        async Task Update()
        {
            message.Text = "Updating from Jazz";
            var dataTask = Task.Run(() => LoadData());
            var sprint = (await Get(new UriBuilder("https://clm1.waters.com/ccm/rpt/repository/foundation")
            {
                Query = $"fields=foundation/developmentLine[name=%27{Uri.EscapeDataString("Acquisition")}%27]/currentIteration/(itemId|startDate|endDate|name)"
            }.Uri)).Elements("foundation").Elements("developmentLine").Elements("currentIteration").FirstOrDefault();
            if (sprint == null)
            {
                throw new ApplicationException("No current sprint not found");
            }
            var sprintId = (string)sprint.Element("itemId");
            var startDate = ParseDateTimeOffset(sprint.Element("startDate"));
            var endDate = ParseDateTimeOffset(sprint.Element("endDate"));
            var sprintName = (string)sprint.Element("name");

            var workItems = (await Get(new UriBuilder("https://clm1.waters.com/ccm/rpt/repository/workitem")
            {
                Query = $"fields=workitem/workItem[target/itemId=%27{Uri.EscapeDataString(sprintId)}%27]/(integerComplexity|state/group)"
            }.Uri)).Elements("workitem").Elements("workItem");

            var data = await dataTask;
            data.Name = sprintName;
            // +1 for the last day
            var rawDuration = (endDate.Date - startDate.Date).Days + 1;
            data.Duration = Enumerable.Range(0, rawDuration).Select(i => startDate.AddDays(i))
                    .Count(d => d.DayOfWeek != DayOfWeek.Saturday && d.DayOfWeek != DayOfWeek.Sunday);

            var checkpoint = new SprintCheckpoint {Time = DateTimeOffset.Now};
            foreach (var item in workItems)
            {
                var points = (int)item.Element("integerComplexity");
                var completed = (string)item.Elements("state").Elements("group").Single() == "closed";
                checkpoint.Total += points;
                if (completed)
                {
                    checkpoint.Done += points;
                }
            }
            data.Checkpoints.RemoveAll(c => c.Time.Date == checkpoint.Time.Date);
            data.Checkpoints.Add(checkpoint);
            var saveTask = Task.Run(() => SaveData(data));

            Render(data);

            await saveTask;
        }

        void Render(SprintData data)
        {
            var model = new PlotModel
            {
                Title = data.Name,
                Axes =
                {
                    new CategoryAxis { Title = "Day", Position = AxisPosition.Bottom },
                    new LinearAxis
                    {
                        Title = "Points",
                        Position = AxisPosition.Left,
                        MinorGridlineStyle = LineStyle.Solid,
                        MajorGridlineStyle = LineStyle.Solid,
                        MinimumPadding = 0.0,
                        MaximumPadding = 0.1
                    }
                },
                IsLegendVisible = true,
                LegendPlacement = LegendPlacement.Outside
            };
            var sustain = new SprintCheckpoint
            {
                Done = 0,
                Total = data.Checkpoints.Select(c => c.Total).LastOrDefault()
            };
            var days = data.Checkpoints.Concat(Enumerable.Repeat(sustain, data.Duration - data.Checkpoints.Count));
            var completedSeries = new ColumnSeries {Title = "Done"};
            foreach (var day in days)
            {
                completedSeries.Items.Add(new ColumnItem(day.Done));
            }
            model.Series.Add(completedSeries);
            var totalSeries = new LineSeries {Title="Scope"};
            var i = 0;
            foreach (var day in days)
            {
                totalSeries.Points.Add(new DataPoint(i++, day.Total));
            }
            model.Series.Add(totalSeries);
            this.plot.Model = model;
        }

        async Task Export()
        {
            message.Text = "Saving";
            var dialog = new SaveFileDialog();
            dialog.OverwritePrompt = true;
            dialog.Filter = "PNG Files|*.png|SVG Files|*.svg";
            dialog.AddExtension = true;
            if (dialog.ShowDialog(this) == true)
            {
                using (var fs = dialog.OpenFile())
                {
                    if (dialog.FileName.EndsWith(".svg", StringComparison.InvariantCultureIgnoreCase))
                    {
                        var exporter = new SvgExporter();
                        exporter.Export(plot.Model, fs);
                    }
                    else
                    {
                        var exporter = new PngExporter();
                        exporter.Export(plot.Model, fs);
                    }
                    fs.SetLength(fs.Position);
                }
            }
        }

        void Refresh_OnExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            BlockInteraction(Update());
            e.Handled = true;
        }

        void Refresh_OnCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !blocked;
            e.ContinueRouting = false;
        }

        void Save_OnExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            BlockInteraction(Export());
            e.Handled = true;
        }

        void Save_OnCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !blocked;
            e.ContinueRouting = false;
        }

        void Copy_OnExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            var data = new DataObject();

            using (var ms = new MemoryStream())
            {
                var pngExporter = new PngExporter();
                pngExporter.Export(plot.Model, ms);

                var bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.StreamSource = ms;
                bitmap.CacheOption = BitmapCacheOption.OnLoad;
                bitmap.EndInit();
                bitmap.Freeze();

                data.SetImage(bitmap);
            }

            var svg = new MemoryStream();
            var svgExporter = new SvgExporter();
            svgExporter.Export(plot.Model, svg);
            svg.Position = 0;
            data.SetData("image/svg+xml", svg);

            Clipboard.SetDataObject(data);
            e.Handled = true;
        }

        void Copy_OnCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !blocked;
            e.ContinueRouting = false;
        }
    }
}
