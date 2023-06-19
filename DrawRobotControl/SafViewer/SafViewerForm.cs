using System.Net;
using System.Windows.Forms;
using static SAFFileHandler.SAFFile;

namespace SafViewer
{
    public partial class SafViewerForm : Form
    {
        // content that we intend to paint on the screen
        SAFTransitionData? SafDataToPaint;
        // allow zooming in/out of content
        float MouseZoom = 1;
        // used to implement dragging of content
        private bool isDragging = false;
        private Point lastCursorPosition;
        // the result of dragging
        int Offset_x = 0;
        int Offset_y = 0;
        // should we flip shown picture vertically or horizontally ?
        bool flipSAFVertically = false;
        bool flipSAFHorizontally = false;
        // so we can add our opened file name to the name of the window
        string OriginalFormTitle;
        int selectedTransitionBlock = 0;
        // some consts
        const float mouseZoomSpeed = 0.1f;
        Color lineColorsSAF = Color.White;
        const float lineWidthSAF = 2;

        public SafViewerForm()
        {
            SafDataToPaint = null;

            InitializeComponent();

            // Subscribe to the Resize event of the form
            Resize += SafViewerForm_Resize;

            // allow drag and drop SAF file opening
            AllowDrop = true;
            DragEnter += SafViewerForm_DragEnter;
            DragDrop += SafViewerForm_DragDrop;

            // Paint the SAF content
            SafContent.Paint += SafContent_Paint;
            // Allow drag and zoom of the SAf file content
            SafContent.MouseWheel += SafContent_MouseWheel;
            SafContent.MouseDown += SafContent_MouseDown;
            SafContent.MouseMove += SafContent_MouseMove;
            SafContent.MouseUp += SafContent_MouseUp;

            // show the file opened right now
            OriginalFormTitle = this.Text;
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            // Resize the form to match the maximum desktop size
            Rectangle screenBounds = Screen.PrimaryScreen.Bounds;
            MaximumSize = new Size(screenBounds.Width, screenBounds.Height);
            WindowState = FormWindowState.Maximized;
        }

        private void SafViewerForm_Resize(object? sender, EventArgs e)
        {
            // Update the size of the PictureBox to match the form size
            SafContent.Size = ClientSize;
            SafContent.Invalidate();
        }

        private void SafViewerForm_DragEnter(object? sender, DragEventArgs e)
        {
            if (e.Data != null && e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                e.Effect = DragDropEffects.Copy;
            }
        }

        private void SafViewerForm_DragDrop(object? sender, DragEventArgs e)
        {
            if( e.Data == null)
            {
                return;
            }

            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if(files.Count() > 1)
            {
                MessageBox.Show("Can only open 1 file at a time");
                return;
            }
            foreach (string file in files)
            {
                SAFFileHandler.SAFFile tSAFFile = new SAFFileHandler.SAFFile();
                int err = tSAFFile.ReadFile(file);
                if(err != 0)
                {
                    MessageBox.Show("Failed to open SAF file");
                    return;
                }
                if( tSAFFile.GetTransitionCount() == 0 )
                {
                    MessageBox.Show("Nothing to draw");
                    return;
                }
                if ( tSAFFile.GetTransitionCount() > 1 )
                {
                    MessageBox.Show("Can only show first transition block");
                }
                selectedTransitionBlock = 0;
                SafDataToPaint = tSAFFile.GetTransitionData(selectedTransitionBlock);
                SafContent.Invalidate();
                this.Text = OriginalFormTitle + " - " + file;
                MouseZoom = 1; // reset zoom
                Offset_x = Offset_y = 0; // reset SAF content offset
                flipSAFHorizontally = false;
                flipSAFVertically = false;
                break;
            }
        }

        private void SafContent_Paint(object? sender, PaintEventArgs e)
        {
            // nothing to paint yet
            if(SafDataToPaint == null || SafDataToPaint.transitionInfo == null)
            {
                return;
            }

            // use our window center to draw the center of the SAF file
            int Offset_X = SafContent.Width / 2 + Offset_x;
            int Offset_Y = SafContent.Height / 2 + Offset_y;

            // scale the content of the SAf file to the size of our window
            float Scale_X = SafContent.Width / SafDataToPaint.transitionInfo.width;
            float Scale_Y = SafContent.Height / SafDataToPaint.transitionInfo.height;
            if(SafDataToPaint.transitionInfo.width == 0)
            {
                Scale_X = 1;
            }
            if(SafDataToPaint.transitionInfo.height == 0)
            {
                Scale_Y = 1;
            }
            // Apply custom zoom to the SAF file content
            Scale_X *= MouseZoom;
            Scale_Y *= MouseZoom;

            // Create a graphics object from the PictureBox
            Graphics g = e.Graphics;

            // Set the color and thickness of the line
            Pen pen = new Pen(lineColorsSAF, lineWidthSAF);

            Point startPoint = new Point(), endPoint = new Point();
            startPoint.X = endPoint.X = 10000;

            foreach (SAFPolyline line in SafDataToPaint.lines)
            {
                foreach (SAFPolylinePoint point in line.points)
                {
                    startPoint = endPoint;
                    float tx = point.x;
                    float ty = point.y;
                    if (flipSAFHorizontally)
                    {
                        ty = SafDataToPaint.transitionInfo.height / 2 - ty;
                    }
                    if (flipSAFVertically)
                    {
                        tx = SafDataToPaint.transitionInfo.width / 2 - tx;
                    }
                    endPoint.X = Offset_X + (int)(tx * Scale_X);
                    endPoint.Y = Offset_Y + (int)(ty * Scale_Y);
                    if (startPoint.X != 10000 && endPoint.X != 10000)
                    {
                        // Draw the line using the start and end points
                        g.DrawLine(pen, startPoint, endPoint);
                    }
                }
            }

            // Dispose of the pen and graphics objects
            pen.Dispose();
        }

        private void SafContent_MouseWheel(object? sender, MouseEventArgs e)
        {
            // Adjust the zoom level based on the direction of the mouse scroll
            if (e.Delta > 0)
            {
                // Zoom in
                MouseZoom += mouseZoomSpeed;
            }
            else if (e.Delta < 0)
            {
                MouseZoom -= mouseZoomSpeed;
            }
            if(MouseZoom < 0)
            {
                MouseZoom = 0;
            }
            SafContent.Invalidate();
        }
        private void SafContent_MouseDown(object? sender, MouseEventArgs e)
        {
            // Start dragging
            isDragging = true;
            lastCursorPosition = e.Location;
        }

        private void SafContent_MouseMove(object? sender, MouseEventArgs e)
        {
            // If dragging is in progress, update the control's position based on the mouse movement
            if (isDragging)
            {
                int dx = e.X - lastCursorPosition.X;
                int dy = e.Y - lastCursorPosition.Y;
                Offset_x += dx;
                Offset_y += dy;
                lastCursorPosition = e.Location;

                SafContent.Invalidate();
            }
        }

        private void SafContent_MouseUp(object? sender, MouseEventArgs e)
        {
            // Stop dragging
            isDragging = false;
        }

        private void flipVerticalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            flipSAFVertically = !flipSAFVertically;
            SafContent.Invalidate();
        }

        private void flipHorizontalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            flipSAFHorizontally = !flipSAFHorizontally;
            SafContent.Invalidate();
        }

        private void selectTransitionBlockToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Create an input dialog form
            GetSAFSelectedTransaction inputDialog = new GetSAFSelectedTransaction();

            // Show the input dialog as a modal dialog
            if (inputDialog.ShowDialog() == DialogResult.OK)
            {
                // Get the user input value
                selectedTransitionBlock = inputDialog.GetSelectedTransition();
            }
        }
    }
}