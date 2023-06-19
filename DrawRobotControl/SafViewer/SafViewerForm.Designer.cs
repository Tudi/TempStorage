namespace SafViewer
{
    partial class SafViewerForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.SafContent = new System.Windows.Forms.PictureBox();
            this.cm_SAFContent = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.flipVerticalToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.flipHorizontalToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectTransitionBlockToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.SafContent)).BeginInit();
            this.cm_SAFContent.SuspendLayout();
            this.SuspendLayout();
            // 
            // SafContent
            // 
            this.SafContent.BackColor = System.Drawing.Color.Black;
            this.SafContent.ContextMenuStrip = this.cm_SAFContent;
            this.SafContent.Location = new System.Drawing.Point(0, 0);
            this.SafContent.Name = "SafContent";
            this.SafContent.Size = new System.Drawing.Size(278, 202);
            this.SafContent.TabIndex = 0;
            this.SafContent.TabStop = false;
            // 
            // cm_SAFContent
            // 
            this.cm_SAFContent.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.flipVerticalToolStripMenuItem,
            this.flipHorizontalToolStripMenuItem,
            this.selectTransitionBlockToolStripMenuItem});
            this.cm_SAFContent.Name = "contextMenuStrip1";
            this.cm_SAFContent.Size = new System.Drawing.Size(191, 92);
            // 
            // flipVerticalToolStripMenuItem
            // 
            this.flipVerticalToolStripMenuItem.Name = "flipVerticalToolStripMenuItem";
            this.flipVerticalToolStripMenuItem.Size = new System.Drawing.Size(190, 22);
            this.flipVerticalToolStripMenuItem.Text = "Flip vertical";
            this.flipVerticalToolStripMenuItem.Click += new System.EventHandler(this.flipVerticalToolStripMenuItem_Click);
            // 
            // flipHorizontalToolStripMenuItem
            // 
            this.flipHorizontalToolStripMenuItem.Name = "flipHorizontalToolStripMenuItem";
            this.flipHorizontalToolStripMenuItem.Size = new System.Drawing.Size(190, 22);
            this.flipHorizontalToolStripMenuItem.Text = "Flip horizontal";
            this.flipHorizontalToolStripMenuItem.Click += new System.EventHandler(this.flipHorizontalToolStripMenuItem_Click);
            // 
            // selectTransitionBlockToolStripMenuItem
            // 
            this.selectTransitionBlockToolStripMenuItem.Name = "selectTransitionBlockToolStripMenuItem";
            this.selectTransitionBlockToolStripMenuItem.Size = new System.Drawing.Size(190, 22);
            this.selectTransitionBlockToolStripMenuItem.Text = "Select transition block";
            this.selectTransitionBlockToolStripMenuItem.Click += new System.EventHandler(this.selectTransitionBlockToolStripMenuItem_Click);
            // 
            // SafViewerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(280, 201);
            this.Controls.Add(this.SafContent);
            this.Name = "SafViewerForm";
            this.Text = "SAF Viewer";
            ((System.ComponentModel.ISupportInitialize)(this.SafContent)).EndInit();
            this.cm_SAFContent.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private PictureBox SafContent;
        private ContextMenuStrip cm_SAFContent;
        private ToolStripMenuItem flipVerticalToolStripMenuItem;
        private ToolStripMenuItem flipHorizontalToolStripMenuItem;
        private ToolStripMenuItem selectTransitionBlockToolStripMenuItem;
    }
}