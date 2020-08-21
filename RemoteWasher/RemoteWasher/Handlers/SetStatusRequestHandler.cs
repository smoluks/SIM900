using MediatR;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;
using RemoteWasher.Handlers.Requests;
using RemoteWasher.Services.Interfaces;
using System;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace RemoteWasher.Handlers
{
    public class SetStatusRequestHandler : AsyncRequestHandler<SetStatusRequest>
    {
        private readonly ApplicationContext _context;
        private readonly ILogger<SetStatusRequestHandler> _logger;
        private readonly ITelegramService _telegramService;

        public SetStatusRequestHandler(ApplicationContext context, ILogger<SetStatusRequestHandler> logger, ITelegramService telegramService)
        {
            _context = context;
            _logger = logger;
            _telegramService = telegramService;
        }

        protected override async Task Handle(SetStatusRequest request, CancellationToken cancellationToken)
        {
            var lastRecord = await _context.DeviceStatuses
                .Where(x => x.DeviceId == request.DeviceId)
                .OrderByDescending(x => x.Timestamp)
                .FirstOrDefaultAsync(cancellationToken);

            _context.DeviceStatuses.Add(new DAO.DeviceStatus()
            {
                DeviceId = request.DeviceId,
                Timestamp = DateTime.Now,
                SygnalLevel = request.SygnalLevel
            });

            await _context.SaveChangesAsync();

            if (lastRecord == null || (DateTime.Now - lastRecord.Timestamp).Minutes > 2)
            {
                await _telegramService.SendMessageAsync($"Washer {request.DeviceId}: connection restored", cancellationToken);
            }
        }
    }
}
