using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SafViewer
{
    public partial class GetSAFSelectedTransaction : Form
    {
        public GetSAFSelectedTransaction()
        {
            InitializeComponent();
        }

        private void b_ConfirmSelection_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }

        public int GetSelectedTransition()
        {
            if (this.tb_SelectedTransition.Text == null)
            {
                return 0;
            }
            try
            {
                return int.Parse(tb_SelectedTransition.Text);
            }
            catch
            {
                return 0;
            }
        }
    }
}
