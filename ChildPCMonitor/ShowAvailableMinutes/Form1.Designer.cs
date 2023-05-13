using Microsoft.Win32;

namespace ShowAvailableMinutes
{
    partial class Form1
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
            this.label1 = new System.Windows.Forms.Label();
            this.lPlayed = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.lMinutesHave = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.lMinutesRemain = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Segoe UI", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(149, 30);
            this.label1.TabIndex = 0;
            this.label1.Text = "MinutesPlayed";
            // 
            // lPlayed
            // 
            this.lPlayed.AutoSize = true;
            this.lPlayed.Font = new System.Drawing.Font("Segoe UI", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.lPlayed.Location = new System.Drawing.Point(167, 9);
            this.lPlayed.Name = "lPlayed";
            this.lPlayed.Size = new System.Drawing.Size(68, 30);
            this.lPlayed.TabIndex = 1;
            this.lPlayed.Text = "label2";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Segoe UI", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.label2.Location = new System.Drawing.Point(12, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(135, 30);
            this.label2.TabIndex = 2;
            this.label2.Text = "MinutesHave";
            // 
            // lMinutesHave
            // 
            this.lMinutesHave.AutoSize = true;
            this.lMinutesHave.Font = new System.Drawing.Font("Segoe UI", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.lMinutesHave.Location = new System.Drawing.Point(167, 54);
            this.lMinutesHave.Name = "lMinutesHave";
            this.lMinutesHave.Size = new System.Drawing.Size(68, 30);
            this.lMinutesHave.TabIndex = 3;
            this.lMinutesHave.Text = "label3";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Segoe UI", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.label3.Location = new System.Drawing.Point(12, 93);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(157, 30);
            this.label3.TabIndex = 4;
            this.label3.Text = "MinutesRemain";
            // 
            // lMinutesRemain
            // 
            this.lMinutesRemain.AutoSize = true;
            this.lMinutesRemain.Font = new System.Drawing.Font("Segoe UI", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.lMinutesRemain.Location = new System.Drawing.Point(167, 93);
            this.lMinutesRemain.Name = "lMinutesRemain";
            this.lMinutesRemain.Size = new System.Drawing.Size(68, 30);
            this.lMinutesRemain.TabIndex = 5;
            this.lMinutesRemain.Text = "label4";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(288, 147);
            this.Controls.Add(this.lMinutesRemain);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.lMinutesHave);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.lPlayed);
            this.Controls.Add(this.label1);
            this.Name = "Form1";
            this.Text = "Stalker v2";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

    #endregion

        private Label label1;
        private Label lPlayed;
        private Label label2;
        private Label lMinutesHave;
        private Label label3;
        private Label lMinutesRemain;
    }
}