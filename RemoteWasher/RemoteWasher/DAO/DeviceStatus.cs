using System;
using System.ComponentModel.DataAnnotations;

namespace RemoteWasher.DAO
{
    public class DeviceStatus
    {
        [Key]
        public int Id { get; set; }

        public int DeviceId { get; set; }

        public DateTime Timestamp { get; set; }

        public int SygnalLevel { get; set; }
    }
}
