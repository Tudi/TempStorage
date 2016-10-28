using System.Collections.Generic;
using System.Runtime.Serialization;

namespace JazzBurnReport
{
    [DataContract]
    class SprintData
    {
        [DataMember]
        public string Name { get; set; }
        [DataMember]
        public int Duration { get; set; }
        [DataMember]
        public List<SprintCheckpoint> Checkpoints { get; set; } = new List<SprintCheckpoint>();
    }
}
