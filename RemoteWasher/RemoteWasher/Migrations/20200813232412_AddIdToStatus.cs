using Microsoft.EntityFrameworkCore.Migrations;
using Npgsql.EntityFrameworkCore.PostgreSQL.Metadata;

namespace RemoteWasher.Migrations
{
    public partial class AddIdToStatus : Migration
    {
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropPrimaryKey(
                name: "PK_DeviceStatuses",
                table: "DeviceStatuses");

            migrationBuilder.AlterColumn<int>(
                name: "DeviceId",
                table: "DeviceStatuses",
                nullable: false,
                oldClrType: typeof(int),
                oldType: "integer")
                .OldAnnotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn);

            migrationBuilder.AddColumn<int>(
                name: "Id",
                table: "DeviceStatuses",
                nullable: false,
                defaultValue: 0)
                .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn);

            migrationBuilder.AddPrimaryKey(
                name: "PK_DeviceStatuses",
                table: "DeviceStatuses",
                column: "Id");
        }

        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropPrimaryKey(
                name: "PK_DeviceStatuses",
                table: "DeviceStatuses");

            migrationBuilder.DropColumn(
                name: "Id",
                table: "DeviceStatuses");

            migrationBuilder.AlterColumn<int>(
                name: "DeviceId",
                table: "DeviceStatuses",
                type: "integer",
                nullable: false,
                oldClrType: typeof(int))
                .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn);

            migrationBuilder.AddPrimaryKey(
                name: "PK_DeviceStatuses",
                table: "DeviceStatuses",
                column: "DeviceId");
        }
    }
}
