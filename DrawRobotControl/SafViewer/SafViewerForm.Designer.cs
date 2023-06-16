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
            this.SafContent = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.SafContent)).BeginInit();
            this.SuspendLayout();
            // 
            // SafContent
            // 
            this.SafContent.BackColor = System.Drawing.Color.Black;
            this.SafContent.Location = new System.Drawing.Point(0, 0);
            this.SafContent.Name = "SafContent";
            this.SafContent.Size = new System.Drawing.Size(278, 202);
            this.SafContent.TabIndex = 0;
            this.SafContent.TabStop = false;
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
            this.ResumeLayout(false);

        }

        #endregion

        private PictureBox SafContent;
    }
}