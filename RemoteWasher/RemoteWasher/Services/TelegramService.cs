using Microsoft.Extensions.Logging;
using RemoteWasher.Services.Interfaces;
using System;
using System.Threading;
using System.Threading.Tasks;
using Telegram.Bot;
using Telegram.Bot.Args;
using Telegram.Bot.Types;

namespace RemoteWasher.Services
{
    public class TelegramService : ITelegramService
    {
        private const string ACCESS_TOKEN = "1085495549:AAEPgyDzBb6MDt5RueE1oGCPKfRV8ZK9N-8";

        private readonly ILogger<TelegramService> _logger;
        TelegramBotClient client;

        public TelegramService(ILogger<TelegramService> logger)
        {
            _logger = logger;

            client = new TelegramBotClient(ACCESS_TOKEN);
            client.OnMessage += OnMessage;
            client.StartReceiving();
        }

        private void OnMessage(object sender, MessageEventArgs e)
        {
            _logger.LogInformation($"User {e.Message.Chat.Id} post: {e.Message.Text}");
        }

        public async Task SendMessageAsync(string text, CancellationToken cancellationToken)
        {
            Message message = await client.SendTextMessageAsync(482040445, text, cancellationToken: cancellationToken);
        }
    }
}
