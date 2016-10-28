using System;
using System.Runtime.Serialization;

namespace JazzBurnReport
{
    [DataContract]
    class SprintCheckpoint
    {
        [DataMember]
        public DateTimeOffset Time { get; set; }
        [DataMember]
        public int Total { get; set; }
        [DataMember]
        public int Done { get; set; }
    }
}