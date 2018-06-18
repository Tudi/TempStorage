using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient
{
    class AbsenceManage
    {
        int StartHour;
        int EndHour;
        int CheckedDays;
        int RefreshTime;
        bool ShowOutlookDialog;
        string OutlookProfile;

        public void SetHours(int Start, int End)
        {
            StartHour = Start;
            EndHour = End;
        }

        public void SetCheckedDays(int days)
        {
            CheckedDays = days;
        }

        public void SetRefreshTime(int refresh)
        {
            RefreshTime = refresh;
        }

        public void SetOutlookSettings(bool Show, string profile)
        {
            ShowOutlookDialog = Show;
            OutlookProfile = profile;
        }
    }
}
