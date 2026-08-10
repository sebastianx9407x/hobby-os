# Port settings reference — 16550 UART (COM1)

Bit-level reference for the serial registers. Concepts and "why" live in
[glossary.md](glossary.md); this file is the lookup table.

Base addresses: **COM1 `0x3F8`**, COM2 `0x2F8`, COM3 `0x3E8`, COM4 `0x2E8`.
All offsets below are from the base.

---

## Register map

Three offsets are overloaded — the same port means different things depending
on the DLAB bit (LCR bit 7) or on whether you read or write.

| Offset | DLAB | Read              | Write                  |
|--------|------|-------------------|------------------------|
| `+0`   | 0    | RBR — receive     | THR — transmit         |
| `+0`   | 1    | DLL — divisor low | DLL — divisor low      |
| `+1`   | 0    | IER               | IER                    |
| `+1`   | 1    | DLM — divisor high| DLM — divisor high     |
| `+2`   | —    | IIR — int. ident. | FCR — FIFO control     |
| `+3`   | —    | LCR               | LCR                    |
| `+4`   | —    | MCR               | MCR                    |
| `+5`   | —    | LSR — line status | (read-only)            |
| `+6`   | —    | MSR — modem status| (read-only)            |
| `+7`   | —    | SR — scratch      | SR — scratch           |

---

## `+1` IER — Interrupt Enable Register

```
bit 0 — data available (a byte arrived)
bit 1 — transmitter holding register empty
bit 2 — line status change / receive error
bit 3 — modem status change
bits 4-7 — reserved (0)
```

`0x00` = never interrupt, poll instead. Required until an IDT exists —
an IRQ before milestone 2 triple faults. Note this is only *half* the gate:
MCR bit 3 (OUT2) must also be set for the IRQ to physically reach the PIC.

---

## `+2` FCR — FIFO Control Register (write)

```
bit 0 — enable FIFOs
bit 1 — clear receive FIFO
bit 2 — clear transmit FIFO
bit 3 — DMA mode select
bits 5-4 — reserved
bits 7-6 — receive trigger level
```

Trigger level: `00`=1 byte, `01`=4, `10`=8, `11`=14.

Enable + clear both = bits 0,1,2 = `0x07`. With a 14-byte trigger: **`0xC7`**.

The FIFO is a 16-byte hardware buffer per direction (the older 8250 had none —
one byte, and a second arrival before you read lost the first). Trigger 14
rather than 16 leaves two bytes of headroom so bytes arriving while the CPU
reaches the handler don't overrun. Inert while IER is `0x00`.

A *character timeout* interrupt also fires when bytes sit in the FIFO and none
arrive for ~4 character times — which is why a high trigger doesn't strand a
lone keypress forever.

---

## `+2` IIR — Interrupt Identification Register (read)

```
bit 0 — interrupt pending  (0 = PENDING — inverted!)
bits 3-1 — interrupt ID
bits 7-6 — FIFO status
```

Read to find out *why* the UART interrupted. Bit 0 is active-low, which catches
people. Not needed until milestone 2.

---

## `+3` LCR — Line Control Register

```
bits 1-0 — data bits: 00=5, 01=6, 10=7, 11=8
bit 2    — stop bits: 0=1, 1=2
bit 3    — parity enable (0 = none; bits 5-4 ignored when clear)
bit 4    — even parity select (0=odd, 1=even)
bit 5    — stick parity
bit 6    — break enable
bit 7    — DLAB
```

**8N1 = `0x03`.** The `3` comes from the data-bits field (`11` = 8 bits); one
stop bit is why bit 2 stays *clear*. 8N2 would be `0x07`.

Two jobs in one register. Bit 7 (DLAB) changes what `+0` and `+1` mean, so
ordering matters: set DLAB → write divisor → write `0x03`, which both applies
8N1 *and* clears DLAB, restoring `+0`/`+1` to data register and IER. Do it the
other way round and the divisor writes land on the data register and the IER.

---

## `+4` MCR — Modem Control Register

```
bit 0 — DTR (Data Terminal Ready)
bit 1 — RTS (Request To Send)
bit 2 — OUT1 (unused on PC)
bit 3 — OUT2 ← on a PC, gates the UART's IRQ line to the PIC
bit 4 — LOOP (loopback mode)
bits 7-5 — reserved
```

`0x0B` = DTR + RTS + OUT2. `0x1E` adds LOOP for the self-test. `0x0F` = normal
operation with loopback off.

**OUT2 is the surprise.** The UART's interrupt output is physically wired
through it on a PC. Clear, and the chip can raise an interrupt internally that
never reaches the PIC.

**Loopback** internally wires transmit straight back to receive — send a known
byte, read it back, compare, and you've proven the chip works before you rely
on it as your only debug channel.

> **Trap:** loopback must be cleared afterwards. Leave bit 4 set and everything
> you print stays inside the chip forever — the self-test passes, init looks
> perfect, and the terminal is silent.

(The OSDev comment on `0x0B` says "DSR"; bit 0 is DTR. DSR is an *input* — the
other end telling you it's ready — and can't be set.)

---

## `+5` LSR — Line Status Register (read-only)

```
bit 0 — data ready (a byte is waiting to be read)
bit 1 — overrun error (a byte was lost before you read it)
bit 2 — parity error
bit 3 — framing error (no valid stop bit — usually a baud mismatch)
bit 4 — break interrupt
bit 5 — transmit holding register empty  ← poll this before writing
bit 6 — transmitter empty (THR and shift register both drained)
bit 7 — error in receive FIFO
```

**Bit 5 is the one that matters for output.** Check it before writing to `+0`
or fast writes overwrite a byte still waiting to go out, and characters vanish.

Bit 5 vs bit 6: bit 5 means "you may hand me another byte"; bit 6 means "the
wire is fully idle". Bit 5 for throughput, bit 6 for waiting until it's truly
flushed (before a reset, say).

Bit 3 (framing error) is the classic symptom of the two ends disagreeing on
baud rate.

---

## `+6` MSR — Modem Status Register (read-only)

```
bit 0 — delta CTS         bit 4 — CTS (Clear To Send)
bit 1 — delta DSR         bit 5 — DSR (Data Set Ready)
bit 2 — trailing edge RI  bit 6 — RI  (Ring Indicator)
bit 3 — delta DCD         bit 7 — DCD (Data Carrier Detect)
```

The low four are "this changed since you last read"; the high four are current
state. Modem-era handshaking — irrelevant under QEMU, and unused for a debug
console.

---

## `+7` SR — Scratch Register

No hardware function; one byte of storage. Its only real use is chip detection —
write a value, read it back, and if it sticks you have a 16450 or later rather
than an original 8250.

---

## Baud divisor

`baud = 115200 / divisor`, written as two bytes (low → `+0`, high → `+1`) with
DLAB set.

| Divisor | Baud   |
|---------|--------|
| 1       | 115200 |
| 2       | 57600  |
| 3       | 38400  |
| 6       | 19200  |
| 12      | 9600   |

Divisor `0` is invalid — division by zero, behavior varies by chip.

Divisor 1 is standard for a kernel debug console (Linux defaults to
`console=ttyS0,115200n8`). Remember it's 16 bits across two ports: writing only
the low byte leaves garbage in the high one.
