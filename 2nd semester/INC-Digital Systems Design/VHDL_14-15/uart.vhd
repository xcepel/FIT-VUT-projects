-- uart.vhd: UART controller - receiving part
-- Author: xcepel03
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------
entity UART_RX is
port(	
  CLK: 	     in std_logic;
	RST: 	     in std_logic;
	DIN: 	     in std_logic;
	DOUT: 	    out std_logic_vector(7 downto 0);
	DOUT_VLD: 	out std_logic
);
end UART_RX;  

-------------------------------------------------

architecture behavioral of UART_RX is
  signal cnt    : std_logic_vector(4 downto 0) := "00001";
  signal cnt2   : std_logic_vector(3 downto 0) := "0000";
  signal cnt3   : std_logic_vector(3 downto 0) := "0000";
  signal cnt_en : std_logic := '0'; -- zapina/vypina pocitadlo
  signal dt_vld : std_logic := '0';
  signal rx_en  : std_logic := '0';
  
begin
  FSM: entity work.UART_FSM(behavioral)
    port map (
        CLK 	    => CLK,
        RST 	    => RST,
        DIN 	    => DIN,
        CNT1	    => cnt, -- pocitadlo hodinovych cyklu
        CNT2    	=> cnt2, -- pocitadlo nactenych bitu
        CNT3     => cnt3, -- pocitadlo hodinovych cyklu stop bitu
        CNT_EN   => cnt_en,
        DT_VLD   => dt_vld,
        RX_EN    => rx_en
    );
    
    process (CLK) begin
      if rising_edge(CLK) then
         DOUT_VLD <= '0'; -- automaticka 0 do DOUT_VLD
        
         if dt_vld = '1' then
           dt_vld <= '0';
         end if;
         
         -- pripocita dalsi hod. cyklus -------
          if CNT_EN = '1' then
             cnt <= cnt + '1';
          end if;  
          
          -- zapnuti pocitadla stop bitu -------
          if cnt2 = "1000" then
            cnt3 <= cnt3 + '1';
          end if;
          
          -- nalezeni stop bitu -------
          if cnt3 = "1000" then 
            -- reset pocitadel do puvodniho stavu -------
            cnt2 <= "0000"; 
            cnt3 <= "0000"; 
            -- validni data -----
            DOUT_VLD <= '1';
          end if;
          
          -- cteni dat a zasilani jich do DOUT -------
          if rx_en = '1' AND cnt(4) = '1' then
                case cnt2 is
                  when "0000" => DOUT(0) <= DIN;
                  when "0001" => DOUT(1) <= DIN;
                  when "0010" => DOUT(2) <= DIN;
                  when "0011" => DOUT(3) <= DIN;
                  when "0100" => DOUT(4) <= DIN;
                  when "0101" => DOUT(5) <= DIN;
                  when "0110" => DOUT(6) <= DIN;
                  when "0111" => DOUT(7) <= DIN;
                  when others => null;
                end case;
                cnt <= "00001"; -- vyresetovani po nacteni 1 bitu
                cnt2 <= cnt2 + '1';  -- pripocita dalsi nacteny bit
          end if;
      end if;  
    end process;
end behavioral;
