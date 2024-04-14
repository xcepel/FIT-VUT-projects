-- uart_fsm.vhd: UART controller - finite state machine
-- Author: xcepel03 
--
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------
entity UART_FSM is
port(
   CLK    : in std_logic;
   RST    : in std_logic;
   DIN    : in std_logic;
   CNT1   : in std_logic_vector(4 downto 0); -- 5 bitove pocitadlo hodinovych cyklu (0-24)
   CNT2   : in std_logic_vector(3 downto 0); -- 3 bitove pocitadlo nactenych bitu (0-7)
   CNT3   : in std_logic_vector(3 downto 0); -- 3 botove pocistlo stop bitu
   CNT_EN : out std_logic;
   DT_VLD : out std_logic;
   RX_EN  : out std_logic
   );
end entity UART_FSM;

-------------------------------------------------
architecture behavioral of UART_FSM is
  type STATE_TYPE is (WAIT_START, WAIT_FIRST, READ_DATA, WAIT_STOP, VALID_DATA);
  signal state : STATE_TYPE := WAIT_START; -- FSM si pamatuje pouze svuj stav
begin
  RX_EN <= '1' when state = READ_DATA else '0';
  CNT_EN <= '1' when state = WAIT_FIRST or state = READ_DATA else '0';
  DT_VLD <= '1' when state = VALID_DATA else '0';
  
  process(CLK) begin
    if rising_edge(CLK) then
        if RST = '1' then
          state <= WAIT_START;

        else
          case state is
            -- 1. stav (WAIT_START)-------
            when WAIT_START   =>  if DIN = '0' then
                                    state <= WAIT_FIRST;
                                  end if;
            -- 2. stav (WAIT_FIRST)-------
            when WAIT_FIRST   =>  if CNT1 = "11000" then
                                    state <= READ_DATA;
                                  end if;
            -- 3. stav  (READ_DATA)-------                   
            when READ_DATA    =>  if CNT2 = "1000" then
                                    state <= WAIT_STOP;
                                  end if;
            -- 4. stav  (WAIT_START)-------                   
            when WAIT_STOP    =>  if CNT3 = "1000" then
                                    state <= VALID_DATA;
                                  end if;
            -- 5. stav  (WAIT_START)-------                   
            when VALID_DATA   =>  state <= WAIT_START;
            when others       =>  null;
          end case;
        end if; 
    end if;
  end process;
end behavioral;