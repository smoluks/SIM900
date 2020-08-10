using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace RemoteWasher.Controllers
{
    [ApiController]

    public class WasherController : ControllerBase
    {
        private readonly ILogger<WasherController> _logger;

        public WasherController(ILogger<WasherController> logger)
        {
            _logger = logger;
        }

        [HttpPost]
        [Route("/{deviceId}/status")]
        public async Task ProcessEventAsync([FromRoute]string deviceId)
        {
            var result = new byte[1]; 
            await Request.Body.ReadAsync(result, 0, 1);

            var level = result[0];

            _logger.LogInformation($"Sygnal level {level}, device {deviceId}");
        }

        [HttpPost]
        [Route("/{deviceId}/card")]
        public async Task<byte> CheckCardAsync(string deviceId)
        {
            byte[] cardId;
            using (var ms = new MemoryStream(5))
            {
                await Request.Body.CopyToAsync(ms);
                cardId = ms.ToArray();
            }

            _logger.LogInformation($"Check card 0x{ByteArrayToString(cardId)}, device {deviceId}");

            return 1;
        }

        public static string ByteArrayToString(byte[] ba)
        {
            StringBuilder hex = new StringBuilder(ba.Length * 2);
            foreach (byte b in ba)
                hex.AppendFormat("{0:x2}", b);

            return hex.ToString();
        }
    }
}
