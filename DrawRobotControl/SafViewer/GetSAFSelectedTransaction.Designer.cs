namespace SafViewer
{
    partial class GetSAFSelectedTransaction
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.b_ConfirmSelection = new System.Windows.Forms.Button();
            this.tb_SelectedTransition = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // b_ConfirmSelection
            // 
            this.b_ConfirmSelection.Location = new System.Drawing.Point(101, 41);
            this.b_ConfirmSelection.Name = "b_ConfirmSelection";
            this.b_ConfirmSelection.Size = new System.Drawing.Size(75, 23);
            this.b_ConfirmSelection.TabIndex = 0;
            this.b_ConfirmSelection.Text = "Select";
            this.b_ConfirmSelection.UseVisualStyleBackColor = true;
            this.b_ConfirmSelection.Click += new System.EventHandler(this.b_ConfirmSelection_Click);
            // 
            // tb_SelectedTransition
            // 
            this.tb_SelectedTransition.Location = new System.Drawing.Point(101, 12);
            this.tb_SelectedTransition.MaxLength = 4;
            this.tb_SelectedTransition.Name = "tb_SelectedTransition";
            this.tb_SelectedTransition.Size = new System.Drawing.Size(75, 23);
            this.tb_SelectedTransition.TabIndex = 1;
            // 
            // GetSAFSelectedTransaction
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(260, 77);
            this.Controls.Add(this.tb_SelectedTransition);
            this.Controls.Add(this.b_ConfirmSelection);
            this.Name = "GetSAFSelectedTransaction";
            this.Text = "Select transition to show";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private Button b_ConfirmSelection;
        private TextBox tb_SelectedTransition;
    }
}