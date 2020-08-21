using MediatR;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using RemoteWasher.Handlers.Requests;
using System.Text;
using System.Threading.Tasks;

namespace RemoteWasher.Controllers
{
    [ApiController]

    public class WasherController : ControllerBase
    {
        private readonly ILogger<WasherController> _logger;
        private readonly IMediator _mediator;

        public WasherController(ILogger<WasherController> logger, IMediator mediator)
        {
            _logger = logger;
            _mediator = mediator;
        }

        [HttpPost]
        [Route("/{deviceId}/status")]
        public async Task ProcessEventAsync([FromRoute]int deviceId)
        {
            var result = new byte[1]; 
            await Request.Body.ReadAsync(result, 0, 1);

            var level = result[0];

            _logger.LogInformation($"Sygnal level {level}, device {deviceId}");

            await _mediator.Send(new SetStatusRequest()
            {
                DeviceId = deviceId,
                SygnalLevel = level
            });
        }

        [HttpPost]
        [Route("/{deviceId}/card")]
        public async Task<IActionResult> CheckCard(string deviceId)
        {
            byte[] cardId = new byte[5];
            await Request.Body.ReadAsync(cardId, 0, 5);

            _logger.LogInformation($"Check card 0x{ByteArrayToString(cardId)}, device {deviceId}");

            return File(new byte[] { 1234 & 0xFF, 1234 >> 8 }, "application/octet-stream");
        }
        [HttpPost]

        [Route("/{deviceId}/card/writeoff")]
        public async Task WriteOffCardAsync(string deviceId)
        {
            byte[] cardId = new byte[7];
            await Request.Body.ReadAsync(cardId, 0, 5);

            byte[] d = new byte[2];
            await Request.Body.ReadAsync(d, 0, 2);
            var duration = d[0] + (d[1] << 8);

            _logger.LogInformation($"Write-off card 0x{ByteArrayToString(cardId)}, duration {duration}, device {deviceId}");
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
