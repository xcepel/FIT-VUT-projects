-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2022 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): jmeno <login AT stud.fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.Numeric_STD.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (0) / zapis (1)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is
-- CNT --
    signal cnt_out              : std_logic_vector(7 downto 0);
    signal cnt_inc, cnt_dec     : std_logic;
-- PC --
    signal pc_addr              : std_logic_vector(12 downto 0);
    signal pc_inc, pc_dec       : std_logic;
-- PTR --
    signal ptr_addr             : std_logic_vector(12 downto 0); --? 12 bitove znaky
    signal ptr_inc, ptr_dec     : std_logic;
-- MX --
    signal sel1                 : std_logic;
--    signal mx1_out              : std_logic_vector(12 downto 0);
    signal sel2                 : std_logic_vector(1 downto 0);
--    signal mx2_out              : std_logic_vector(7 downto 0);
-- STATES
    type fsm_state is (
    -- Hlavni   
        s_START,
        s_FETCH,
        s_DECODE,
    --  Prikazy/Execute
        s_RIGHT,     -- >
        s_LEFT,      -- <
        s_PLUS,      -- + add 1
        s_MX2_01,
        s_PLUS_END,
        s_MINUS,     -- - remove 1
        s_MINUS_READ,
        s_MX2_10,
        s_MINUS_END,
        s_DOT,       -- . write
        s_DOT_LOOP,
        s_DOT_END,
        s_COMMA,     -- , read
        s_READ,
        s_READING,
        s_COMMA_END,
        s_L_SQ,      -- [
        s_L_SQ_NEXT,
        s_L_SQ_LOAD,
        s_L_SQ_LOOP,
        s_R_SQ,      -- ]
        s_R_SQ_LOOP,
        s_L_BR,
        s_R_BR,      -- )
        s_R_BR_NEXT,
        s_R_BR_LOAD,
        s_R_BR_LOOP,
        s_NULL       -- null
    );
    signal present_state : fsm_state := s_START;
    signal next_state : fsm_state;

begin

-- CNT --    
    CNT: process (RESET, CLK)
    begin
        if (RESET = '1') then
            cnt_out <= (others => '0'); --vse na 0
        elsif (CLK'event and CLK = '1') then
            if (cnt_inc = '1') then
                cnt_out <= cnt_out + 1;
            elsif (cnt_dec = '1') then
                cnt_out <= cnt_out - 1;
            end if;
        end if;
    end process CNT;

-- PC --
    PC: process (RESET, CLK)
    begin
        if (RESET = '1') then
            pc_addr <= (others => '0'); --vse na 0
        elsif (CLK'event and CLK = '1') then
            if (pc_inc = '1') then
                pc_addr <= pc_addr + 1;
            elsif (pc_dec = '1') then
                pc_addr <= pc_addr - 1;
            end if;
        end if;
    end process PC;

-- PTR --
    PTR: process (RESET, CLK)
    begin -- PTR <= 0x1000 + (PTR + 1) % 0x1000?
        if (RESET = '1') then
            ptr_addr <= "1000000000000"; -- 0x1000
        elsif (CLK'event and CLK = '1') then
            if (ptr_inc = '1') then
                if (ptr_addr = x"1FFF") then
                    ptr_addr <= "1000000000000";
                else
                    ptr_addr <= ptr_addr + 1;
                end if;
            elsif (ptr_dec = '1') then
                if (ptr_addr = x"1000") then
                    ptr_addr <= "1111111111111";
                else
                    ptr_addr <= ptr_addr - 1;
                end if;
            end if;
        end if;
    end process PTR;

-- Multiplexory --
    MX1: process(RESET, CLK, sel1)
    begin
        if (RESET = '1') then
            DATA_ADDR <= (others => '0');
        elsif (CLK'event and CLK = '1') then
            case sel1 is
                when '0' => DATA_ADDR <= pc_addr;
                when '1' => DATA_ADDR <= ptr_addr;
                when others => DATA_ADDR <= (others => '0');
            end case;
        end if;
    end process MX1;

    MX2: process(RESET, CLK, sel2)
    begin
        if (RESET = '1') then
            DATA_WDATA <= (others => '0');
        elsif (CLK'event and CLK = '1') then
            case sel2 is
                when "00" => DATA_WDATA <= IN_DATA;
                when "10" => DATA_WDATA <= DATA_RDATA - 1;
                when "01" => DATA_WDATA <= DATA_RDATA + 1;
                when "11" => OUT_DATA <= DATA_RDATA;
                when others => DATA_WDATA <= (others => '0');
            end case;
        end if;
    end process MX2;

-- FSM --
    state_logic: process(RESET, CLK, EN)
    begin
        if (RESET = '1') then
            present_state <= s_START;
        elsif (CLK'event and CLK = '1') then
            if (EN = '1') then
                present_state <= next_state;
            end if;
        end if;
    end process state_logic;

    FSM: process (RESET, CLK, EN, OUT_BUSY, IN_VLD)
    begin
    
    -- inicializace
        cnt_inc <= '0';
        cnt_dec <= '0';

        pc_inc <= '0';
        pc_dec <= '0';     
    
        ptr_inc <= '0';
        ptr_dec <= '0';
        
        OUT_WE <= '0';
        DATA_EN <= '0';
        DATA_RDWR <= '0'; -- read
        
        if (RESET = '1') then
            IN_REQ <= '0';
        end if;
    -- Stavy
        case present_state is
            when s_START => 
                OUT_WE <= '0';
                sel1 <= '0';
                sel2 <= "00";
                next_state <= s_FETCH;
            when s_FETCH =>
                DATA_EN <= '1'; --read implicitne
                next_state <= s_DECODE;
            when s_DECODE => 
                case DATA_RDATA is
                    when x"3E" => next_state <= s_RIGHT;
                    when x"3C" => next_state <= s_LEFT;
                    when x"2B" =>
                        sel1 <= '1';
                        next_state <= s_PLUS;
                    when x"2D" =>
                        sel1 <= '1';
                        next_state <= s_MINUS;
                    when x"2E" =>
                        sel1 <= '1';
                        next_state <= s_DOT;
                    when x"2C" => next_state <= s_COMMA;
                    when x"5B" => 
                        next_state <= s_L_SQ;
                        sel1 <= '1'; -- ptr
                        pc_inc <= '1';
                    when x"5D" => 
                        next_state <= s_R_SQ;
                        sel1 <= '0'; -- pc
                        pc_dec <= '1';
                        cnt_dec <= '1';
                    when x"28" => next_state <= s_L_BR;
                    when x"29" => 
                        next_state <= s_R_BR;
                        cnt_dec <= '1';
                        sel1 <= '1'; -- ptr
                       -- pc_inc <= '1';
                    when x"00" => next_state <= s_NULL;
                    when others =>
                        pc_inc <= '1';
                        next_state <= s_START; -- skip neznamych znaku
                end case;
            -- posunuti ptr doprava
            when s_RIGHT =>
                ptr_inc <= '1';
                pc_inc <= '1';
                next_state <= s_START;
            -- posunuti ptr doleva
            when s_LEFT =>
                ptr_dec <= '1';
                pc_inc <= '1';
                next_state <= s_START;
            -- pricteni hodnoty 1
            when s_PLUS =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                next_state <= s_MX2_01;
            when s_MX2_01 =>
                sel2 <= "01";
                next_state <= s_PLUS_END;
            when s_PLUS_END =>
                DATA_RDWR <= '1';
                DATA_EN <= '1';
                pc_inc <= '1';
                next_state <= s_START;
            -- odecteni hodnoty 1
            when s_MINUS =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                next_state <= s_MX2_10;
            when s_MX2_10 =>
                sel2 <= "10";
                next_state <= s_MINUS_END;
            when s_MINUS_END =>
                DATA_RDWR <= '1';
                DATA_EN <= '1';
                pc_inc <= '1';
                next_state <= s_START;
            -- print aktualni pozice 
            when s_DOT =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                next_state <= s_DOT_LOOP;
            when s_DOT_LOOP =>
                if (OUT_BUSY = '0') then
                    sel2 <= "11";
                    next_state <= s_DOT_END;
                else
                    next_state <= s_DOT_LOOP;
                end if;
            when s_DOT_END =>
                pc_inc <= '1';
                OUT_WE <= '1'; 
                next_state <= s_START;
            -- nacteni vstupu z klavesnice
            when s_COMMA =>
                sel1 <= '1';
                IN_REQ <= '1';
                next_state <= s_READ;
            when s_READ =>
                IN_REQ <= '1';
                if (IN_VLD = '1') then
                    sel2 <= "00";
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    next_state <= s_COMMA_END;
                else
                    next_state <= s_READ;
                end if;
            when s_COMMA_END =>
                DATA_RDWR <= '1';
                DATA_EN <= '1';
                pc_inc <= '1';
                IN_REQ <= '0';
                next_state <= s_START;
            -- leva zavorka WHILE loopu
            when s_L_SQ =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                sel1 <= '1'; -- pc
                pc_inc <= '1';
                next_state <= s_L_SQ_NEXT;
            when s_L_SQ_NEXT =>
                if (DATA_RDATA = "00000000") then -- while skoncil
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    sel1 <= '0'; -- pc
                    pc_dec <= '1';
                    next_state <= s_L_SQ_LOOP;
                else
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    pc_dec <= '1';
                    next_state <= s_START; -- while pokracuje
                end if;
            when s_L_SQ_LOAD =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                sel1 <= '0';
                next_state <= s_L_SQ_LOOP;
            when s_L_SQ_LOOP =>
                if (DATA_RDATA = x"5B") then
                    cnt_inc <= '1';
                end if;
                if (DATA_RDATA /= x"5D") then -- skip veci mezi [ ]
                    sel1 <= '0'; -- pc
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    next_state <= s_L_SQ_LOAD;
                    pc_inc <= '1';
                else -- skip ]
                    if (cnt_out = "00000000") then
                        next_state <= s_START;
                    else 
                        sel1 <= '0'; -- pc
                        DATA_EN <= '1';
                        DATA_RDWR <= '0';
                        pc_inc <= '1';
                        cnt_dec <= '1';
                        next_state <= s_L_SQ_LOAD;
                    end if;
                end if;
            -- prava zavorka WHILE loopu
            when s_R_SQ =>
                --pc_dec <= '0';
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                sel1 <= '0'; -- pc
                next_state <= s_R_SQ_LOOP;
            when s_R_SQ_LOOP =>
                if (DATA_RDATA = x"5B" AND cnt_out = "00000000") then
                    pc_inc <= '1';
                    next_state <= s_START;
                else
                    if (DATA_RDATA = x"5B") then
                        cnt_dec <= '1';
                    elsif (DATA_RDATA = x"5D") then
                        cnt_inc <= '1';
                    end if;
                    sel1 <= '0'; -- pc
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    pc_dec <= '1';
                    next_state <= s_R_SQ;
                end if;
            -- leva zavorka DO WHILE loopu
            when s_L_BR =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                pc_inc <= '1';
                next_state <= s_START;
            -- prava zavorka DO WHILE loopu
            when s_R_BR =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                next_state <= s_R_BR_NEXT;
                sel1 <= '1'; -- pc
            when s_R_BR_NEXT =>
                if (DATA_RDATA = "00000000") then -- while skoncil
                    DATA_EN <= '1'; --?
                    pc_inc <= '1';
                    DATA_RDWR <= '0'; --?
                  --  sel1 <= '0'; -- pc
                    if (cnt_out = "11111111") then
                        cnt_inc <= '1';
                    end if;
                    next_state <= s_START;
                else
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    sel1 <= '0'; -- pc
                    pc_dec <= '1';
                    next_state <= s_R_BR_LOAD; -- while pokracuje
                end if;
            when s_R_BR_LOAD =>
                --pc_dec <= '0';
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                sel1 <= '0'; -- pc
                next_state <= s_R_BR_LOOP;
            when s_R_BR_LOOP =>
                if (DATA_RDATA = x"28" AND cnt_out = "00000000") then
                    pc_inc <= '1';
                    next_state <= s_START;
                else
                    if (DATA_RDATA = x"28") then
                        cnt_dec <= '1';
                    elsif (DATA_RDATA = x"29") then
                        cnt_inc <= '1';
                    end if;
                    sel1 <= '0'; -- pc
                    DATA_EN <= '1';
                    DATA_RDWR <= '0';
                    pc_dec <= '1';
                    next_state <= s_R_BR_LOAD;
                end if;
            when s_NULL => 
            when others => null;
        end case;
    end process FSM;

end behavioral;

